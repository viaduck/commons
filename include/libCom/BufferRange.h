#ifndef LIBCOM_BUFFERRANGE_H
#define LIBCOM_BUFFERRANGE_H

#include <string>

#include <libCom/Range.h>

class Buffer;
typedef Range<Buffer> BufferRange;
typedef Range<const Buffer> BufferRangeConst;

#include <libCom/Buffer.h>

// taken from https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
namespace std {

    /**
     * Implement hash function for Range class (so that Range is usable in a hash map)
     */
    template<>
    struct hash<const BufferRangeConst> {
        std::size_t operator()(const BufferRangeConst &k) const {
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
                        static_cast<const uint8_t *>(k.const_object().const_data())[k.offset() + a] + 0x9e3779b9 +
                        (current << 6) + (current >> 2);
            return current;
        }
    };
}


#endif //LIBCOM_BUFFERRANGE_H
