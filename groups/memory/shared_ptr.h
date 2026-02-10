#pragma once

#include <cstddef>

namespace ksl {
template <typename T> class shared_ptr {
    struct control_block {
        T *ptr;
        size_t ref_count;
    };

    control_block *cb;

    void release() {
        if (cb) {
            --cb->ref_count;
            if (cb->ref_count == 0) {
                delete cb->ptr;
                delete cb;
            }
            cb = nullptr;
        }
    }

  public:
    // Constructor
    /// Default constructor that contains no managed object and zero
    /// shared references
    constexpr shared_ptr() noexcept;

    constexpr shared_ptr(std::nullptr_t) noexcept;

    /// Creates a shared_ptr that manages the given raw pointer.
    explicit shared_ptr(T *ptr);

    // Copy constructor
    shared_ptr(const shared_ptr<T> &rhs);

    // Move constructor
    shared_ptr(shared_ptr<T> &&rhs) = delete;

    // Copy assignment
    /// Assignment operator that deletes the current managed object and
    /// takes ownership of the given shared_ptr's managed object.
    shared_ptr<T> &operator=(const shared_ptr<T> &rhs);

    // Move assignment
    shared_ptr<T> &operator=(shared_ptr<T> &&rhs) = delete;

    // Desctructor
    /// Destroys the managed object if this is the last shared_ptr owning it.
    ~shared_ptr();

  public:
    // ACCESSORS
    [[nodiscard]] T &operator*() const noexcept { return *(cb->ptr); }

    [[nodiscard]] T *operator->() const noexcept { return cb->ptr; }
};

template <typename T>
constexpr shared_ptr<T>::shared_ptr() noexcept : cb(new control_block{nullptr, 0}) {}

template <typename T>
constexpr shared_ptr<T>::shared_ptr(std::nullptr_t) noexcept : cb(new control_block{nullptr, 0}) {}

template <typename T> shared_ptr<T>::shared_ptr(T *ptr) : cb(new control_block{ptr, 1}) {}

template <typename T> shared_ptr<T>::~shared_ptr() { release(); }

template <typename T> shared_ptr<T>::shared_ptr(const shared_ptr<T> &rhs) : cb(rhs.cb) {
    ++cb->ref_count;
}

template <typename T> shared_ptr<T> &shared_ptr<T>::operator=(const shared_ptr<T> &rhs) {
    if (this != &rhs) {
        release();
        this->cb = rhs.cb;
        this->cb->ref_count++;
    }
    return *this;
}
} // namespace ksl