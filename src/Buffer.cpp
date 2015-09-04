#include <string.h>
#include "SecureUniquePtr.h"
#include "Buffer.h"

Buffer::Buffer(uint32_t reserved) : mData(reserved), mReserved(reserved) {
    //mData = new uint8_t[reserved];
}

Buffer::Buffer(const Buffer &buffer) : mData(buffer.mReserved), mReserved(buffer.mReserved), mUsed(buffer.mUsed), mOffset(0) {
    // TODO copy the buffer

    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    //memcpy(newData, (mData+mOffset), mUsed);
    memcpy(mData().get(), &buffer.mData()[buffer.mOffset], mUsed);
}

Buffer::~Buffer() {
    // TODO secure delete
    //delete[] mData;
}

void Buffer::append(const void *data, uint32_t len) {

    if (mOffset+mUsed+len > mReserved) {
        increase(mOffset+mUsed+len + mReserved*2);

        // now copy new data
        memcpy(&mData()[mUsed], data, len);
    } else {
        memcpy(&mData()[mOffset+mUsed], data, len);
    }
    mUsed += len;
}

void Buffer::append(const char *data, uint32_t len) {
    append(static_cast<const void *>(data), len);
}

void Buffer::consume(uint32_t n) {
    if (n > mUsed)
        n = mUsed;
    mOffset += n;
    mUsed -= n;
}

void Buffer::increase(uint32_t newSize) {
    // reallocate
    mReserved = newSize;
    //uint8_t *newData = new uint8_t[mReserved];
    SecureUniquePtr<uint8_t[]> newData(mReserved);

    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    //memcpy(newData, (mData+mOffset), mUsed);
    memcpy(newData().get(), &mData()[mOffset], mUsed);
    // TODO secure delete
    //delete[] mData;

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
