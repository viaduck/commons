// byte order (network-byte-order <-> host-byte-order) conversions
// network byte order is big-endian, therefore we can use be*toh und htobe* methods


#ifndef PUSHCLIENT_CONVERSIONS_H
#define PUSHCLIENT_CONVERSIONS_H

extern "C" {
    #include "libCom/conversions_crossplatform.h"
};

/**
 * Converts an uint8_t from network to host byte order
 * @param v Value to convert to network byte order
 * @return v in host byte order
 */
inline uint8_t ntoh_uint8_t(uint8_t v) {
    // a byte is a byte, no need to swap
    return v;
}

/**
 * Converts an uint16_t from network to host byte order
 * @param v Value to convert to network byte order
 * @return v in host byte order
 */
inline uint16_t ntoh_uint16_t(uint16_t v) {
    return be16toh(v);
}

/**
 * Converts an uint32_t from network to host byte order
 * @param v Value to convert to network byte order
 * @return v in host byte order
 */
inline uint32_t ntoh_uint32_t(uint32_t v) {
    return be32toh(v);
}

/**
 * Converts an uint64_t from network to host byte order
 * @param v Value to convert to network byte order
 * @return v in host byte order
 */
inline uint64_t ntoh_uint64_t(uint64_t v) {
    return be64toh(v);
}

/**
 * Converts an uint8_t from host to network byte order
 * @param v Value to convert to host byte order
 * @return v in network byte order
 */
inline uint8_t hton_uint8_t(uint8_t v) {
    // a byte is a byte, no need to swap
    return v;
}

/**
 * Converts an uint16_t from host to network byte order
 * @param v Value to convert to host byte order
 * @return v in network byte order
 */
inline uint16_t hton_uint16_t(uint16_t v) {
    return htobe16(v);
}

/**
 * Converts an uint32_t from host to network byte order
 * @param v Value to convert to host byte order
 * @return v in network byte order
 */
inline uint32_t hton_uint32_t(uint32_t v) {
    return htobe32(v);
}

/**
 * Converts an uint64_t from host to network byte order
 * @param v Value to convert to host byte order
 * @return v in network byte order
 */
inline uint64_t hton_uint64_t(uint64_t v) {
    return htobe64(v);
}


#endif //PUSHCLIENT_CONVERSIONS_H
