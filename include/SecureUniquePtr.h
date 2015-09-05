//
// Created by steffen on 20.07.15.
//

#ifndef PUSHCLIENT_AUTOPTR_H
#define PUSHCLIENT_AUTOPTR_H

#include <memory>
#include <iostream>


volatile void *spc_memset(volatile void *dst, unsigned char c, size_t len);


template<typename T>
class SecureUniquePtr {
public:
    SecureUniquePtr() : ptr(new T()) { }

    ~SecureUniquePtr() {
        spc_memset(ptr.get(), 0xDE, sizeof(T));
    }

    std::unique_ptr<T, std::default_delete<T>> &operator()() {
        return ptr;
    }


    const std::unique_ptr<T, std::default_delete<T>> &operator()() const {
        return ptr;
    }

    SecureUniquePtr<T> &operator=(SecureUniquePtr<T> &&other) {
        ptr = std::move(other.ptr);

        return *this;
    }

private:
    std::unique_ptr<T, std::default_delete<T>> ptr;
};

template<typename T>
class SecureUniquePtr<T[]> {
public:
    SecureUniquePtr(size_t size) : mSize(size), ptr(new T[size]) { }

    SecureUniquePtr(SecureUniquePtr<T[]> && other) {
        ptr = std::move(other.ptr);
        mSize = other.mSize;
    }

    ~SecureUniquePtr() {
        spc_memset(ptr.get(), 0x12, sizeof(T)*mSize);
    }

    std::unique_ptr<T[], std::default_delete<T[]>> &operator()() {
        return ptr;
    }

    const std::unique_ptr<T[], std::default_delete<T[]>> &operator()() const {
        return ptr;
    }

    SecureUniquePtr<T[]> &operator=(SecureUniquePtr<T[]> &&other) {
        ptr = std::move(other.ptr);
        mSize = other.mSize;

        other.mSize = 0;
        return *this;
    }

    const size_t size() const {
        return mSize;
    }

private:
    size_t mSize;
    std::unique_ptr<T[], std::default_delete<T[]>> ptr;
};

#endif //PUSHCLIENT_AUTOPTR_H
