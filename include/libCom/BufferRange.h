#ifndef LIBCOM_BUFFERRANGE_H
#define LIBCOM_BUFFERRANGE_H

class Buffer;

#include "libCom/Buffer.h"

class BufferRange {

public:
    BufferRange(const Buffer &buffer, uint32_t size, uint32_t offset);

    bool operator==(const BufferRange &other);

private:
    const Buffer &mBuffer;
    const uint32_t mSize;
    const uint32_t mOffset;
};

#endif //LIBCOM_BUFFERRANGE_H
