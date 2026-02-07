#pragma once

#include <cstddef>

namespace ksl {
template <typename T> class shared_ptr {
    struct control_block {
        T *ptr;
        size_t ref_count;
    };

    control_block *cb;

  public:
    // Constructor
    /// Creates a shared_ptr that manages the given raw pointer.
    explicit shared_ptr(T *ptr = nullptr);

    // Copy constructor
    shared_ptr(const shared_ptr<T> &rhs) = delete;

    // Move constructor
    shared_ptr(shared_ptr<T> &&rhs) = delete;

    // Copy assignment
    /// Assignment operator that deletes the current managed object and
    /// takes ownership of the given shared_ptr's managed object.
    shared_ptr<T> &operator=(const shared_ptr<T> &rhs) = delete;

    // Move assignment
    shared_ptr<T> &operator=(shared_ptr<T> &&rhs) = delete;

    // Desctructor
    /// Destroys the managed object if this is the last shared_ptr owning it.
    ~shared_ptr();

  public:
    // ACCESSORS
    T &operator*() { return *(cb->ptr); }

    T *operator->() { return cb->ptr; }
};

template <typename T> shared_ptr<T>::shared_ptr(T *ptr) : cb(new control_block{ptr, 1}) {}

template <typename T> shared_ptr<T>::~shared_ptr() {
    if (cb) {
        --cb->ref_count;
        if (cb->ref_count == 0) {
            delete cb->ptr;
            delete cb;
        }
    }
}
} // namespace ksl