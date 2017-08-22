#ifndef VDCOMMONS_HELPER_H
#define VDCOMMONS_HELPER_H

#include <cstring>
#include <functional>
#include <secure_memory/conversions.h>

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

/**
 * Helper method for std::hash<> specialisations to combine a proper hash value
 * @tparam T Type of value to hash
 * @param seed Current hash value, will be updated
 * @param v Value to hash
 */
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

#endif //VDCOMMONS_HELPER_H
