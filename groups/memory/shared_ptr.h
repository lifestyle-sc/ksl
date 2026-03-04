#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>

namespace ksl {
namespace {
struct control_block_base {
    size_t shared_count{1};
    size_t weak_count{0};
    virtual void incrementSharedCount() { ++shared_count; };
    virtual void decrementSharedCount() { --shared_count; };
    virtual size_t sharedCount() { return shared_count; };
    virtual void incrementWeakCount() { ++weak_count; };
    virtual void decrementWeakCount() { --weak_count; };
    virtual size_t weakCount() { return weak_count; };
    virtual void dispose() = 0;
    virtual ~control_block_base() = default;
};

template <typename T, typename Deleter = std::default_delete<T>>
struct control_block_impl : public control_block_base {
    T *d_ptr;
    Deleter d_deleter;

    control_block_impl(T *ptr, Deleter deleter) : d_ptr(ptr), d_deleter(deleter) {}
    void dispose() override { d_deleter(d_ptr); }
};
} // namespace

// ============================================================================
// SHARED POINTERS DEFINITION
// ============================================================================

template <typename T> class weak_ptr;

template <typename T> class shared_ptr {
    T *d_ptr;
    control_block_base *d_cb;

    inline void release() {
        if (!d_cb) {
            return;
        }

        d_cb->decrementSharedCount();
        if (d_cb->sharedCount() == 0) {
            d_cb->dispose();
            if (d_cb->weakCount() == 0) {
                delete d_cb;
            }
        }
        d_cb = nullptr;
        d_ptr = nullptr;
    }

  public:
    using Value_Type = T;

  public:
    // Constructor
    /// Default constructor that contains no managed object and zero
    /// shared references
    constexpr shared_ptr() noexcept;

    constexpr shared_ptr(std::nullptr_t) noexcept;

    /// Creates a shared_ptr that manages the given raw pointer.
    explicit shared_ptr(T *ptr);

    /// Creates a shared_ptr that manages the given raw pointer and
    /// a deleter to manage the clean up of the raw pointer
    template <typename Deleter> explicit shared_ptr(T *ptr, Deleter deleter);

    // Create a shared_ptr from a weak_ptr
    explicit shared_ptr(const weak_ptr<T> &ptr);

    // Copy constructor
    shared_ptr(const shared_ptr<T> &rhs) noexcept;

    // Move constructor
    shared_ptr(shared_ptr<T> &&rhs) noexcept;

    // Copy assignment
    /// Assignment operator that deletes the current managed object and
    /// takes ownership of the given shared_ptr's managed object.
    shared_ptr<T> &operator=(const shared_ptr<T> &rhs);

    // Move assignment
    shared_ptr<T> &operator=(shared_ptr<T> &&rhs);

    // Desctructor
    /// Destroys the managed object if this is the last shared_ptr owning it.
    ~shared_ptr();

  private:
    // A private constructor for internal use by weak_ptr
    shared_ptr(T *ptr, control_block_base *cb);

  public:
    // ACCESSORS
    [[nodiscard]] inline T *get() const noexcept { return d_ptr; }

    [[nodiscard]] T &operator*() const noexcept {
        assert(d_ptr != nullptr && "Attempted to dereference a null shared_ptr");
        return *(get());
    }

    [[nodiscard]] T *operator->() const noexcept { return get(); }

    [[nodiscard]] operator bool() const noexcept { return get() != nullptr; };

  public:
    // OBSERVERS
    [[nodiscard]] inline std::size_t use_count() const noexcept {
        if (d_cb) {
            return d_cb->sharedCount();
        }
        return 0;
    }

  public:
    inline void reset() { release(); }

    inline void reset(T *ptr) {
        control_block_impl<T, std::default_delete<T>> *new_cb = nullptr;
        if (ptr != nullptr) {
            new_cb =
                new control_block_impl<T, std::default_delete<T>>{ptr, std::default_delete<T>()};
        }

        release();
        d_cb = new_cb;
        d_ptr = ptr;
    }

    template <typename Deleter> inline void reset(T *ptr, Deleter deleter) {
        control_block_impl<T, Deleter> *new_cb = nullptr;
        if (ptr != nullptr) {
            new_cb = new control_block_impl<T, Deleter>{ptr, deleter};
        }

        release();
        d_cb = new_cb;
        d_ptr = ptr;
    }

    inline void swap(shared_ptr &ptr) noexcept {
        std::swap(this->d_ptr, ptr.d_ptr);
        std::swap(this->d_cb, ptr.d_cb);
    }

  public:
    // Friend
    friend class weak_ptr<T>;
};

// ============================================================================
// WEAK POINTERS DEFINITION
// ============================================================================
template <typename T> class weak_ptr {
    T *d_ptr;
    control_block_base *d_cb;

  private:
    inline void release() {
        if (!d_cb) {
            return;
        }
        d_cb->decrementWeakCount();
        if (d_cb->sharedCount() == 0 && d_cb->weakCount() == 0) {
            delete d_cb;
        }
        d_cb = nullptr;
        d_ptr = nullptr;
    }

    [[nodiscard]] inline T *get() const noexcept { return d_ptr; }

  public:
    using Value_Type = T;

  public:
    /// CONSTRUCTORS
    constexpr weak_ptr() noexcept;

    weak_ptr(const weak_ptr<T> &ptr) noexcept;

    weak_ptr(const shared_ptr<T> &ptr) noexcept;

    weak_ptr(weak_ptr<T> &&ptr) noexcept;

    /// DESTRUCTORS
    ~weak_ptr() noexcept;

    /// ASSIGNMENT
    weak_ptr<T> &operator=(const weak_ptr<T> &rhs) noexcept;

    weak_ptr<T> &operator=(const shared_ptr<T> &rhs) noexcept;

    weak_ptr<T> &operator=(weak_ptr<T> &&rhs) noexcept;

  public:
    /// MODIFIERS
    inline void reset() noexcept { release(); }

    inline void swap(weak_ptr<T> &ptr) noexcept;

  public:
    // OBSERVERS
    [[nodiscard]] inline std::size_t use_count() const noexcept {
        if (d_cb) {
            return d_cb->sharedCount();
        }
        return 0;
    }

    [[nodiscard]] inline bool expired() const noexcept {
        return d_cb == nullptr || d_cb->sharedCount() == 0;
    }

    [[nodiscard]] inline shared_ptr<T> lock() const noexcept {
        return expired() ? shared_ptr<T>() : shared_ptr<T>(this->d_ptr, this->d_cb);
    }

  public:
    // Friend
    friend class shared_ptr<T>;
};

// ============================================================================
// SHARED POINTERS IMPLEMENTATION
// ============================================================================

template <typename T>
constexpr shared_ptr<T>::shared_ptr() noexcept : d_ptr(nullptr), d_cb(nullptr) {}

template <typename T>
constexpr shared_ptr<T>::shared_ptr(std::nullptr_t) noexcept : d_ptr(nullptr), d_cb(nullptr) {}

template <typename T>
shared_ptr<T>::shared_ptr(T *ptr)
    : d_ptr(ptr),
      d_cb(new control_block_impl<T, std::default_delete<T>>{ptr, std::default_delete<T>()}) {}

template <typename T>
template <typename Deleter>
shared_ptr<T>::shared_ptr(T *ptr, Deleter deleter)
    : d_ptr(ptr), d_cb(new control_block_impl<T, Deleter>{ptr, deleter}) {}

template <typename T> shared_ptr<T>::shared_ptr(const weak_ptr<T> &wptr) {
    if (wptr.d_cb && wptr.d_cb->sharedCount() > 0) {
        this->d_ptr = wptr.d_ptr;
        this->d_cb = wptr.d_cb;
        this->d_cb->incrementSharedCount();
    } else {
        this->d_ptr = nullptr;
        this->d_cb = nullptr;
    }
}

template <typename T> shared_ptr<T>::~shared_ptr() { release(); }

template <typename T>
shared_ptr<T>::shared_ptr(const shared_ptr<T> &rhs) noexcept : d_ptr(rhs.d_ptr), d_cb(rhs.d_cb) {
    if (d_cb) {
        d_cb->incrementSharedCount();
    }
}

template <typename T>
shared_ptr<T>::shared_ptr(shared_ptr<T> &&rhs) noexcept : d_ptr(nullptr), d_cb(nullptr) {
    std::swap(this->d_cb, rhs.d_cb);
    std::swap(this->d_ptr, rhs.d_ptr);
}

template <typename T> shared_ptr<T> &shared_ptr<T>::operator=(const shared_ptr<T> &rhs) {
    if (this != &rhs) {
        release();
        this->d_cb = rhs.d_cb;
        this->d_ptr = rhs.d_ptr;
        if (this->d_cb) {
            this->d_cb->incrementSharedCount();
        }
    }
    return *this;
}

template <typename T> shared_ptr<T> &shared_ptr<T>::operator=(shared_ptr<T> &&rhs) {
    if (this != &rhs) {
        release();
        std::swap(this->d_cb, rhs.d_cb);
        std::swap(this->d_ptr, rhs.d_ptr);
    }
    return *this;
}

template <typename T>
shared_ptr<T>::shared_ptr(T *ptr, control_block_base *cb) : d_ptr(ptr), d_cb(cb) {
    if (d_ptr && d_cb) {
        d_cb->incrementSharedCount();
    }
}

// ============================================================================
// WEAK POINTERS IMPLEMENTATION
// ============================================================================

template <typename T> constexpr weak_ptr<T>::weak_ptr() noexcept : d_ptr(nullptr), d_cb(nullptr) {}

template <typename T>
weak_ptr<T>::weak_ptr(const weak_ptr<T> &ptr) noexcept : d_ptr(ptr.d_ptr), d_cb(ptr.d_cb) {
    if (d_cb) {
        d_cb->incrementWeakCount();
    }
}

template <typename T>
weak_ptr<T>::weak_ptr(const shared_ptr<T> &ptr) noexcept : d_ptr(ptr.d_ptr), d_cb(ptr.d_cb) {
    if (d_cb) {
        d_cb->incrementWeakCount();
    }
}

template <typename T>
weak_ptr<T>::weak_ptr(weak_ptr<T> &&ptr) noexcept : d_ptr(nullptr), d_cb(nullptr) {
    std::swap(d_ptr, ptr.d_ptr);
    std::swap(d_cb, ptr.d_cb);
}

template <typename T> weak_ptr<T>::~weak_ptr() noexcept { release(); }

template <typename T> weak_ptr<T> &weak_ptr<T>::operator=(const weak_ptr<T> &rhs) noexcept {
    release();
    this->d_ptr = rhs.d_ptr;
    this->d_cb = rhs.d_cb;
    if (this->d_cb) {
        d_cb->incrementWeakCount();
    }
    return *this;
}

template <typename T> weak_ptr<T> &weak_ptr<T>::operator=(const shared_ptr<T> &rhs) noexcept {
    release();
    this->d_ptr = rhs.d_ptr;
    this->d_cb = rhs.d_cb;
    if (this->d_cb) {
        d_cb->incrementWeakCount();
    }
    return *this;
}

template <typename T> weak_ptr<T> &weak_ptr<T>::operator=(weak_ptr<T> &&rhs) noexcept {
    release();
    std::swap(this->d_ptr, rhs.d_ptr);
    std::swap(this->d_cb, rhs.d_cb);
    return *this;
}

template <typename T> void weak_ptr<T>::swap(weak_ptr<T> &ptr) noexcept {
    std::swap(this->d_ptr, ptr.d_ptr);
    std::swap(this->d_cb, ptr.d_cb);
}
} // namespace ksl