//
// Created by steffen on 12.11.15.
//

#ifndef LIBCOM_HELPER_H
#define LIBCOM_HELPER_H

#include <cstring>
#include "libCom/conversions.h"

inline bool comparisonHelper(const void *one, const void *two, uint32_t size) {
    const char *cthis = static_cast<const char *>(one),
            *cother = static_cast<const char *>(two);

    // loop breaks on different character or if size() elements have been checked
    for (; size != 0 && *cthis == *cother; cthis++, cother++, size--);

    return size == 0;       // if they equal, iteration count equals size
}

inline size_t strlen_s(const char *str) {
    if (str == nullptr)
        return 0;
    return strlen(str);
}

template<char... digits>
struct conv2bin;

template<char high, char... digits>
struct conv2bin<high, digits...> {
    static_assert(high == '0' || high == '1', "no bin num!");
    static int const value = (high - '0') * (1 << sizeof...(digits)) +
                             conv2bin<digits...>::value;
};

template<char... digits>
constexpr uint64_t operator "" _b() {
    return conv2bin<digits...>::value;
}

template<char high>
struct conv2bin<high> {
    static_assert(high == '0' || high == '1', "no bin num!");
    static uint64_t const value = (high - '0');
};



#endif //LIBCOM_HELPER_H
