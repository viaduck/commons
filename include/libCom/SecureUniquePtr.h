#ifndef PUSHCLIENT_AUTOPTR_H
#define PUSHCLIENT_AUTOPTR_H

#include <memory>
#include <iostream>

/**
 * memset(..) with will not be optimezed away by compilers (hopefully) used for memory erasing.
 */
volatile void *sec_memset(volatile void *dst, unsigned char c, size_t len);


/**
 * Wrapper around std::unique_ptr<T> which features secure memory erasing
 */
template<typename T>
class SecureUniquePtr {
public:
    /**
     * Creates a std::unique_ptr<T>
     */
    SecureUniquePtr() : ptr(new T()) { }

    /**
     * Securely overwrite memory used by std::unique_ptr<T>
     */
    ~SecureUniquePtr() {
#ifdef OPTION_SECURE_UNIQUEPTR
        sec_memset(ptr.get(), 0xDE, sizeof(T));
#endif
    }

    /**
     * Convenience method for getting internal std::unique_ptr<T>
     * @return Internal std::unique_ptr<T>
     */
    std::unique_ptr<T, std::default_delete<T>> &operator()() {
        return ptr;
    }

    /**
     * Convenience method for getting internal std::unique_ptr<T> (const)
     * @return Internal std::unique_ptr<T> (const)
     */
    const std::unique_ptr<T, std::default_delete<T>> &operator()() const {
        return ptr;
    }

    /**
     * Transfers ownership of other's std::unique_ptr<T> to us
     * @param other
     */
    SecureUniquePtr<T> &operator=(SecureUniquePtr<T> &&other) {
        ptr = std::move(other.ptr);

        return *this;
    }

private:
    std::unique_ptr<T, std::default_delete<T>> ptr;
};

/**
 * Wrapper around std::unique_ptr<T[]> which features secure memory erasing. This is the array version.
 */
template<typename T>
class SecureUniquePtr<T[]> {
public:
    /**
     * Creates a std::unique_ptr<T[]> with size elements.
     * @param size
     */
    SecureUniquePtr(size_t size) : mSize(size), ptr(new T[size]) { }

    /**
     * Transfers ownership of other's std::unique_ptr<T[]> to us
     * @param other
     */
    SecureUniquePtr(SecureUniquePtr<T[]> && other) {
        ptr = std::move(other.ptr);
        mSize = other.mSize;
    }

    /**
     * Securely overwrite memory used by std::unique_ptr<T[]>
     */
    ~SecureUniquePtr() {
        sec_memset(ptr.get(), 0x12, sizeof(T) * mSize);
    }

    /**
     * Convenience method for getting internal std::unique_ptr<T[]>
     * @return Internal std::unique_ptr<T[]>
     */
    std::unique_ptr<T[], std::default_delete<T[]>> &operator()() {
        return ptr;
    }

    /**
     * Convenience method for getting internal std::unique_ptr<T> (const)
     * @return Internal std::unique_ptr<T> (const)
     */
    const std::unique_ptr<T[], std::default_delete<T[]>> &operator()() const {
        return ptr;
    }

    /**
     * Transfers ownership of other's std::unique_ptr<T[]> to us
     * @param other
     */
    SecureUniquePtr<T[]> &operator=(SecureUniquePtr<T[]> &&other) {
        ptr = std::move(other.ptr);
        mSize = other.mSize;

        other.mSize = 0;
        return *this;
    }

    /**
     * Getter: array's number of elements
     */
    const size_t size() const {
        return mSize;
    }

private:
    size_t mSize;
    std::unique_ptr<T[], std::default_delete<T[]>> ptr;
};

#endif //PUSHCLIENT_AUTOPTR_H
