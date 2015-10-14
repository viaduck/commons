#ifndef PUSHCLIENT_BUFFER_H
#define PUSHCLIENT_BUFFER_H

#include <libCom/SecureUniquePtr.h>
#include <cinttypes>

class BufferRange;
class DevNull;

class Buffer {
public:
    static DevNull DEV_NULL;


    Buffer(uint32_t reserved = 512);
    Buffer(const Buffer &);
    ~Buffer();

    virtual BufferRange append(const void *data, uint32_t len);
    virtual BufferRange append(const char *data, uint32_t len);

    // consumes some bytes from the beginning
    virtual void consume(uint32_t n);

    // resets mOffset by offsetDiff, increases mUsed by offsetDiff
    void reset(uint32_t offsetDiff = 0);

    // increase buffer size
    void increase(const uint32_t newSize);
    void increase(const uint32_t newSize, const uint8_t value);

    const uint32_t size() const;

    void *data();
    void *data(uint32_t p);

    const void *const_data() const;
    const void *const_data(uint32_t p) const;

    const BufferRange const_data(uint32_t offset, uint32_t size) const;

    virtual void use(uint32_t used);

    void clear();

private:
    SecureUniquePtr<uint8_t[]> mData;
    uint32_t mReserved;
    uint32_t mUsed = 0;
    uint32_t mOffset = 0;
};

// fix IDE quick fix tooltip, there is no other reason in doing this besides that
#include "DevNull.h"

#endif //PUSHCLIENT_BUFFER_H
