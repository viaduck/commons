#ifndef PUSHCLIENT_BUFFER_H
#define PUSHCLIENT_BUFFER_H

#include <libCom/SecureUniquePtr.h>
#include <cinttypes>

class BufferRange;

class Buffer {
public:
    Buffer(uint32_t reserved = 512);
    Buffer(const Buffer &);
    ~Buffer();

    BufferRange append(const void *data, uint32_t len);
    BufferRange append(const char *data, uint32_t len);

    // consumes some bytes from the beginning
    void consume(uint32_t n);

    // increase buffer size
    void increase(uint32_t newSize);

    const uint32_t size() const;

    void *data();
    void *data(uint32_t p);

    const void *const_data() const;
    const void *const_data(uint32_t p) const;

    const BufferRange const_data(uint32_t offset, uint32_t size) const;

    void use(uint32_t used);

    void clear();

private:
    SecureUniquePtr<uint8_t[]> mData;
    uint32_t mReserved;
    uint32_t mUsed = 0;
    uint32_t mOffset = 0;
};

#endif //PUSHCLIENT_BUFFER_H
