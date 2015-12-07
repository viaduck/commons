#include "libCom/helper.h"
#include "libCom/BufferRange.h"


BufferRange::BufferRange(const Buffer &buffer, const uint32_t offset, const uint32_t size)
        : mBuffer(buffer), mSize(size), mOffset(offset) { }


bool BufferRange::operator==(const BufferRange &other) const{
    if (other.mSize != mSize)       // size is different -> they are truly not equal
        return false;

    const char *cthis = static_cast<const char *>(mBuffer.const_data(mOffset)),
            *cother = static_cast<const char *>(other.mBuffer.const_data(other.mOffset));

    return comparisonHelper(cthis, cother, mSize);
}
