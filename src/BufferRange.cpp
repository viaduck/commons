#include "libCom/BufferRange.h"


BufferRange::BufferRange(const Buffer &buffer, const uint32_t size, const uint32_t offset) : mBuffer(buffer), mSize(size), mOffset(offset) { }


bool BufferRange::operator==(const BufferRange &other) const{
    if (other.mSize != mSize)
        return false;

    const unsigned char *other_data = static_cast<const unsigned char*>(other.mBuffer.const_data(other.mOffset));
    const unsigned char *this_data = static_cast<const unsigned char*>(mBuffer.const_data(mOffset));

    for (uint32_t i = 0; i < other.mSize; ++i) {
        if (other_data[i] != this_data[i])
            return false;
    }

    return true;
}
