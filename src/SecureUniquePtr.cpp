//
// Created by steffen on 20.07.15.
//

#include "libCom/SecureUniquePtr.h"

volatile void *spc_memset(volatile void *dst, unsigned char c, size_t len) {
    volatile char *buf;

    for (buf = (volatile char *) dst; len; buf[--len] = c);
    return dst;
}
