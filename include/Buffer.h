#ifndef PUSHCLIENT_BUFFER_H
#define PUSHCLIENT_BUFFER_H

#include <SecureUniquePtr.h>
#include <cinttypes>


class Buffer {
public:
    Buffer(uint32_t reserved = 512);
    Buffer(const Buffer &);
    ~Buffer();

    void append(const void *data, uint32_t len);
    void append(const char *data, uint32_t len);

    // consumes some bytes from the beginning
    void consume(uint32_t n);

    // increase buffer size
    void increase(uint32_t newSize);

    const uint32_t size() const;

    void *data();
    void *data(uint32_t p);

    const void *const_data() const;
    const void *const_data(uint32_t p) const;

private:
    SecureUniquePtr<uint8_t[]> mData;
    uint32_t mReserved;
    uint32_t mUsed = 0;
    uint32_t mOffset = 0;
};

#endif //PUSHCLIENT_BUFFER_H
