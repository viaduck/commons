#ifndef LIBCOM_BUFFERRANGE_H
#define LIBCOM_BUFFERRANGE_H

//class Buffer;

#include "libCom/Buffer.h"

class BufferRange {

public:
    /**
     * Creates a BufferRange defining a range (offset, size) within a Buffer object.
     * @param buffer Buffer object
     * @param offset Offset in Buffer to start at
     * @param size Range's size
     */
    BufferRange(const Buffer &buffer, const uint32_t offset, const uint32_t size);

    /**
     * Compares two BufferRanges
     * @param other
     * @return True if content within the ranges is the same
     */
    bool operator==(const BufferRange &other) const;

    /**
     * Getter: Buffer
     * @return buffer
     */
    inline const Buffer &const_buffer() const {
        return mBuffer;
    }

    /**
     * Getter: size
     * @return size
     */
    inline const uint32_t size() const {
        return mSize;
    }

    /**
     * Getter: offset
     * @return offset
     */
    inline const uint32_t offset() const {
        return mOffset;
    }

    /**
     * Getter: underlying data (of Buffer)
     */
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

    /**
     * Implement hash function for BufferRange class (so that BufferRange is usable in a hash map)
     */
    template<>
    struct hash<const BufferRange> {
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
