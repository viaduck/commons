#include <string.h>
#include <libCom/Buffer.h>
#include <libCom/BufferRange.h>

DevNull Buffer::DEV_NULL;

Buffer::Buffer(uint32_t reserved) : mData(reserved), mReserved(reserved) { }

Buffer::Buffer(const Buffer &buffer) : mData(buffer.mReserved), mReserved(buffer.mReserved), mUsed(buffer.mUsed), mOffset(0) {
    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    memcpy(mData().get(), &buffer.mData()[buffer.mOffset], mUsed);
}

Buffer::~Buffer() { }

BufferRangeConst Buffer::append(const void *data, uint32_t len) {
    return write(data, len, mUsed);
}

BufferRangeConst Buffer::append(const char *data, uint32_t len) {
    return append(static_cast<const void *>(data), static_cast<uint32_t>(len*sizeof(char)));
}

BufferRangeConst Buffer::append(const Buffer &other) {
    return append(other.const_data(), other.size());
}

BufferRangeConst Buffer::append(const BufferRangeConst &range) {
    return append(range.const_data(), range.size());
}

BufferRangeConst Buffer::write(const void *data, uint32_t len, uint32_t offset) {
    if (mOffset+offset+len > mReserved)
        increase(mOffset+offset+len + mReserved*2);

    // now copy new data (if not nullptr)
    if (data != nullptr)
        memcpy(&mData()[mOffset+offset], data, len);

    if (offset+len > mUsed)
        mUsed = offset+len;

    return BufferRangeConst(*this, offset, len);
}

BufferRangeConst Buffer::write(const Buffer &other, uint32_t offset) {
    return write(other.const_data(), other.size(), offset);
}

BufferRangeConst Buffer::write(const BufferRangeConst &other, uint32_t offset) {
    return write(other.const_data(), other.size(), offset);
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

uint32_t Buffer::increase(const uint32_t newCapacity, const bool by) {
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

uint32_t Buffer::increase(const uint32_t newCapacity, const uint8_t value, const bool by) {
    uint32_t r = increase(newCapacity, by);
    // initialize with supplied value
    for (uint32_t i = mUsed; i < r; ++i)
        static_cast<uint8_t *>(data())[i] = value;

    return r;
}

void Buffer::padd(const uint32_t offset, const uint32_t size, const uint8_t value) {
    increase(offset+size, value);
    // do not overwrite existing bytes with supplied value (bytes with offset < mUsed). New bytes have been set to value
    // already by increase(..)

    // mark them as used
    if (offset+size > mUsed)
        use(offset+size-mUsed);
}

void Buffer::padd(const uint32_t newSize, const uint8_t value) {
    if (newSize > mUsed)
        padd(mUsed, newSize-mUsed, value);
}

uint32_t Buffer::size() const {
    return mUsed;
}

void *Buffer::data(uint32_t p) {
    if (p > size())
        p = size();
    return &mData()[mOffset+p];
}

const void *Buffer::const_data(uint32_t p) const {
    if (p > size())
        p = size();
    return const_cast<const uint8_t *>(&mData()[mOffset+p]);
}

const BufferRangeConst Buffer::const_data(uint32_t offset, uint32_t size) const {
    if (offset > this->size())
        offset = this->size();
    if (offset+size > this->size())
        size = this->size() - offset;
    return BufferRangeConst(*this, offset, size);
}

void Buffer::use(uint32_t n) {
    if (mReserved >= n+mUsed)
        mUsed += n;
    else
        mUsed = mReserved;
}

void Buffer::unuse(uint32_t n) {
    if (n > mUsed)
        n = mUsed;
    mUsed -= n;
}

void Buffer::clear() {
    mUsed = 0;
    mOffset = 0;
}

bool Buffer::operator==(const Buffer &other) const {
    return size() == other.size() && comparisonHelper(const_data(), other.const_data(), this->size());
}
