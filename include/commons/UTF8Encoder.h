#ifndef UTF8ENCODER_H
#define UTF8ENCODER_H

#include <secure_memory/String.h>

class UTF8Encoder {
public:
    void encode(String &result, uint32_t codepoint) {
        // content is bigger than 4-byte-utf8 can hold
        if ((codepoint & 0xFFE00000) > 0)
            return;

        // content fits into 1-byte-utf8
        if ((codepoint & 0xFFFFFF80) == 0) {
            uint8_t firstByte = 0x80 | codepoint;

            result.append(&firstByte, sizeof(firstByte));
        }
        // fits 2-byte-utf8
        else if((codepoint & 0xFFFFF800) == 0) {
            uint8_t firstByte = 0xC0 | ((codepoint & 0x7C0) >> 6);
            uint8_t secondByte = 0x80 | (codepoint & 0x3F);

            result.append(&firstByte, sizeof(firstByte));
            result.append(&secondByte, sizeof(secondByte));
        }
        // fits 3-byte-utf8
        else if((codepoint & 0xFFFF0000) == 0) {
            uint8_t firstByte = 0xE0 | ((codepoint & 0xF000) >> 12);
            uint8_t secondByte = 0x80 | ((codepoint & 0xFC0) >> 6);
            uint8_t thirdByte = 0x80 | (codepoint & 0x3F);

            result.append(&firstByte, sizeof(firstByte));
            result.append(&secondByte, sizeof(secondByte));
            result.append(&thirdByte, sizeof(thirdByte));
        }
        // fits 4-byte-utf8
        else {
            uint8_t firstByte = 0xF0 | ((codepoint & 0x1C0000) >> 18);
            uint8_t secondByte = 0x80 | ((codepoint & 0x3F000) >> 12);
            uint8_t thirdByte = 0x80 | ((codepoint & 0xFC0) >> 6);
            uint8_t fourthByte = 0x80 | (codepoint & 0x3F);

            result.append(&firstByte, sizeof(firstByte));
            result.append(&secondByte, sizeof(secondByte));
            result.append(&thirdByte, sizeof(thirdByte));
            result.append(&fourthByte, sizeof(fourthByte));
        }
    }
};

#endif // UTF8ENCODER_H
