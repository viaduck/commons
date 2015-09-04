/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/

#ifndef sometest_H
#define sometest_H


#include <Buffer.h>
#include <ThinXchange/conversions.h>


class sometest {
public:
    sometest(const Buffer &buffer) : mBuffer(buffer) {
        if (mBuffer.size() < SIZE)
            mBuffer.increase(SIZE);     // prevent access resulting in SIGSEGV if buffer is too small
    }

    const Buffer &buffer() const {
        return mBuffer;
    }

    // ++++++++ ///

    // - version - //
    inline const uint32_t version() const {
        return hton_uint32_t(*static_cast<const uint32_t*>(mBuffer.const_data(0)));
    }
    inline void version(uint32_t v) {
        *static_cast<uint32_t*>(mBuffer.data(0)) = hton_uint32_t(v);
    }

    // - first - //
    inline const uint8_t first() const {
        return hton_uint8_t(*static_cast<const uint8_t*>(mBuffer.const_data(4)));
    }
    inline void first(uint8_t v) {
        *static_cast<uint8_t*>(mBuffer.data(4)) = hton_uint8_t(v);
    }

    // - second - //
    inline const uint16_t second() const {
        return hton_uint16_t(*static_cast<const uint16_t*>(mBuffer.const_data(5)));
    }
    inline void second(uint16_t v) {
        *static_cast<uint16_t*>(mBuffer.data(5)) = hton_uint16_t(v);
    }

    // - buf - //
    inline const uint8_t* buf_const() const {
        return static_cast<const uint8_t*>(mBuffer.const_data(7));
    }
    inline uint8_t* buf() {
        return static_cast<uint8_t*>(mBuffer.data(7));
    }

    // - third - //
    inline const uint16_t third() const {
        return hton_uint16_t(*static_cast<const uint16_t*>(mBuffer.const_data(17)));
    }
    inline void third(uint16_t v) {
        *static_cast<uint16_t*>(mBuffer.data(17)) = hton_uint16_t(v);
    }


    // ++++++++ ///

    const uint32_t SIZE = 19;

private:
    Buffer mBuffer;
};

#endif //sometest_H
