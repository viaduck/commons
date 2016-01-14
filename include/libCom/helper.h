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

#endif //LIBCOM_HELPER_H
