#ifndef LIBCOM_BUFFERRANGE_H
#define LIBCOM_BUFFERRANGE_H

//class Buffer;

#include "libCom/Buffer.h"

class BufferRange {

public:
    BufferRange(const Buffer &buffer, const uint32_t size, const uint32_t offset);

    bool operator==(const BufferRange &other) const;


    inline const Buffer &const_buffer() const {
        return mBuffer;
    }

    inline const uint32_t size() const {
        return mSize;
    }

    inline const uint32_t offset() const {
        return mOffset;
    }

    inline const void *const_data() const {
        return const_buffer().const_data(offset());
    }

private:
    const Buffer &mBuffer;
    const uint32_t mSize;
    const uint32_t mOffset;
};

// taken from https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
namespace std {

    template<>
    struct hash<BufferRange> {
        std::size_t operator()(const BufferRange &k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            // taken from https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
            size_t current = 0;
            for (uint32_t a = 0; a < k.size(); a++)
                current ^=
                        static_cast<const uint8_t *>(k.const_buffer().const_data())[k.offset() + a] + 0x9e3779b9 +
                        (current << 6) + (current >> 2);
            return current;
        }
    };
}

#endif //LIBCOM_BUFFERRANGE_H
