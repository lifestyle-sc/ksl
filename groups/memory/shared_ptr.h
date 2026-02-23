#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>

namespace ksl {
namespace {
struct control_block_base {
    size_t ref_count{1};
    virtual void incrementRefCount() { ++ref_count; };
    virtual void decrementRefCount() { --ref_count; };
    virtual size_t refCount() { return ref_count; };
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
template <typename T> class shared_ptr {
    T *d_ptr;
    control_block_base *d_cb;

    inline void release() {
        if (d_cb) {
            d_cb->decrementRefCount();
            if (d_cb->refCount() == 0) {
                d_cb->dispose();
                delete d_cb;
            }
            d_cb = nullptr;
            d_ptr = nullptr;
        }
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
    [[nodiscard]] inline long use_count() const noexcept {
        if (d_cb) {
            return d_cb->refCount();
        }
        return 0;
    }

  public:
    inline void reset() { release(); }

    inline void reset(T *ptr) {
        release();
        d_cb = new control_block_impl<T, std::default_delete<T>>{ptr, std::default_delete<T>()};
        d_ptr = ptr;
    }

    template <typename Deleter> inline void reset(T *ptr, Deleter deleter) {
        release();
        d_cb = new control_block_impl<T, Deleter>{ptr, deleter};
        d_ptr = ptr;
    }
};

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

template <typename T> shared_ptr<T>::~shared_ptr() { release(); }

template <typename T>
shared_ptr<T>::shared_ptr(const shared_ptr<T> &rhs) noexcept : d_ptr(rhs.d_ptr), d_cb(rhs.d_cb) {
    if (d_ptr) {
        d_cb->incrementRefCount();
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
            this->d_cb->incrementRefCount();
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
} // namespace ksl