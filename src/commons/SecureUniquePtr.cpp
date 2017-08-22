//
// Created by steffen on 20.07.15.
//

#include "commons/SecureUniquePtr.h"

volatile void *SecureUniquePtrPRNG::shred(volatile void *dst, size_t len) {
    if (dst == nullptr)
        return dst;

    volatile char *buf;

    for (buf = (volatile char *) dst; len; buf[--len] = SecureUniquePtrPRNG::get());
    return dst;
}

thread_local std::minstd_rand SecureUniquePtrPRNG::mRandGenerator(std::random_device().operator()());
thread_local std::uniform_int_distribution<uint8_t> SecureUniquePtrPRNG::mRandDistribution(std::numeric_limits<uint8_t>::min(),std::numeric_limits<uint8_t>::max());
