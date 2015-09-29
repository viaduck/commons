#include <string.h>
#include <libCom/BufferRange.h>

Buffer::Buffer(uint32_t reserved) : mData(reserved), mReserved(reserved) { }

Buffer::Buffer(const Buffer &buffer) : mData(buffer.mReserved), mReserved(buffer.mReserved), mUsed(buffer.mUsed), mOffset(0) {
    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    memcpy(mData().get(), &buffer.mData()[buffer.mOffset], mUsed);
}

Buffer::~Buffer() { }

BufferRange Buffer::append(const void *data, uint32_t len) {

    if (mOffset+mUsed+len > mReserved) {
        increase(mOffset+mUsed+len + mReserved*2);

        // now copy new data
        memcpy(&mData()[mUsed], data, len);
    } else {
        memcpy(&mData()[mOffset+mUsed], data, len);
    }
    mUsed += len;

    return BufferRange(*this, len, mUsed-len);
}

BufferRange Buffer::append(const char *data, uint32_t len) {
    return append(static_cast<const void *>(data), len);
}

void Buffer::consume(uint32_t n) {
    if (n > mUsed)
        n = mUsed;
    mOffset += n;
    mUsed -= n;
}

void Buffer::reset(uint32_t offsetDiff) {
    if (offsetDiff > mOffset)
        offsetDiff = 0;
    mUsed += offsetDiff;
    mOffset -= offsetDiff;
}

void Buffer::increase(uint32_t newSize) {
    // no need to increase, since buffer is as big as requested
    if (newSize <= mReserved)
        return;

    // reallocate
    mReserved = newSize;
    SecureUniquePtr<uint8_t[]> newData(mReserved);

    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    memcpy(newData().get(), &mData()[mOffset], mUsed);

    mData = std::move(newData);
    mOffset = 0;
}

const uint32_t Buffer::size() const {
    return mUsed;
}

void *Buffer::data() {
    return &mData()[mOffset];
}

void *Buffer::data(uint32_t p) {
    return &mData()[mOffset+p];
}

const void *Buffer::const_data() const {
    return const_cast<const uint8_t *>(&mData()[mOffset]);
}

const void *Buffer::const_data(uint32_t p) const {
    return const_cast<const uint8_t *>(&mData()[mOffset+p]);
}

const BufferRange Buffer::const_data(uint32_t offset, uint32_t size) const {
    return BufferRange(*this, size, offset);
}

void Buffer::use(uint32_t used) {
    if (mReserved >= used+mUsed)
        mUsed += used;
}

void Buffer::clear() {
    mUsed = 0;
    mOffset = 0;
}
