#ifndef LIBCOM_DEVNULL_H
#define LIBCOM_DEVNULL_H


#include "Buffer.h"

/**
 * Subclass of Buffer which noops some operations, behaving like a /dev/null
 */
class DevNull : public Buffer {

public:
    BufferRange append(const void *data, uint32_t len) override;
    BufferRange append(const char *data, uint32_t len) override;
    void consume(uint32_t n) override;
    void use(uint32_t used) override;
};


#endif //LIBCOM_DEVNULL_H
