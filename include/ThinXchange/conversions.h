// byte order (network-byte-order <-> host-byte-order) conversions
// network byte order is big-endian, therefore we can use be*toh und htobe* methods


#ifndef PUSHCLIENT_CONVERSIONS_H
#define PUSHCLIENT_CONVERSIONS_H

#include <netinet/in.h>


// -- network -> host --
inline uint8_t ntoh_uint8_t(uint8_t v) {
    // a byte is a byte, no need to swap
    return v;
}

inline uint16_t ntoh_uint16_t(uint16_t v) {
    return be16toh(v);
}

inline uint32_t ntoh_uint32_t(uint32_t v) {
    return be32toh(v);
}

inline uint64_t ntoh_uint64_t(uint64_t v) {
    return be64toh(v);
}

// -- host -> network --
inline uint8_t hton_uint8_t(uint8_t v) {
    // a byte is a byte, no need to swap
    return v;
}

inline uint16_t hton_uint16_t(uint16_t v) {
    return htobe16(v);
}

inline uint32_t hton_uint32_t(uint32_t v) {
    return htobe32(v);
}

inline uint64_t hton_uint64_t(uint64_t v) {
    return htobe64(v);
}


#endif //PUSHCLIENT_CONVERSIONS_H
