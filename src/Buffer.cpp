#include <string.h>
#include <libCom/BufferRange.h>

DevNull Buffer::DEV_NULL;

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

BufferRange Buffer::append(const Buffer &other) {
    return append(other.const_data(), other.size());
}

BufferRange Buffer::append(const BufferRange &range) {
    return append(range.const_data(), range.size());
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

const uint32_t Buffer::increase(const uint32_t newCapacity, const bool by) {
    uint32_t capa = newCapacity;
    if (by)
        capa += size();
    // no need to increase, since buffer is as big as requested
    if (capa <= (mReserved-mOffset))
        return (mReserved-mOffset);

    // reallocate
    mReserved = capa;
    SecureUniquePtr<uint8_t[]> newData(mReserved);

    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    memcpy(newData().get(), &mData()[mOffset], mUsed);

    mData = std::move(newData);
    mOffset = 0;

    return mReserved;
}

const uint32_t Buffer::increase(const uint32_t newCapacity, const uint8_t value, const bool by) {
    uint32_t r = increase(newCapacity, by);
    // initialize with supplied value
    for (uint32_t i = mUsed; i < r; ++i)
        static_cast<uint8_t *>(data())[i] = value;

    return r;
}

void Buffer::padd(const uint32_t newSize, const uint8_t value) {
    increase(newSize, value);
    // mark them as used
    use(newSize-mUsed);
}

const uint32_t Buffer::size() const {
    return mUsed;
}

void *Buffer::data(uint32_t p) {
    return &mData()[mOffset+p];
}

const void *Buffer::const_data(uint32_t p) const {
    return const_cast<const uint8_t *>(&mData()[mOffset+p]);
}

const BufferRange Buffer::const_data(uint32_t offset, uint32_t size) const {
    return BufferRange(*this, size, offset);
}

void Buffer::use(uint32_t n) {
    if (mReserved >= n+mUsed)
        mUsed += n;
}

void Buffer::clear() {
    mUsed = 0;
    mOffset = 0;
}
