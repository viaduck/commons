#ifndef PUSHCLIENT_BUFFER_H
#define PUSHCLIENT_BUFFER_H

#include <libCom/SecureUniquePtr.h>
#include <cinttypes>

class BufferRange;
class DevNull;

class Buffer {
public:
    /**
     * Static allocated DevNull Buffer object for all your garbage!
     */
    static DevNull DEV_NULL;

    /**
     * Creates a Buffer object with an internal buffer of reserved size.
     * @param reserved Initial buffer capacity in bytes. Defaults to 512
     */
    Buffer(uint32_t reserved = 512);

    /**
     * Creates a Buffer object from another Buffer (deep-copy).
     * @param buffer A reference to the buffer to be copied
     */
    Buffer(const Buffer &buffer);

    /**
     * Destructor
     */
    ~Buffer();

    /**
     * Appends a bunch of data to the Buffer; increases it's capacity if necessary.
     *
     * Capacity is at least double the capacity before.
     * @param data Data pointer
     * @param len Length of data (in bytes)
     */
    virtual BufferRange append(const void *data, uint32_t len);
    /**
     * Overloaded variant of append(const void *data, uint32_t len) which accepts char* for convenience.
     * @param data Data pointer
     * @param len Length of data (in bytes)
     */
    virtual BufferRange append(const char *data, uint32_t len);

    /**
     * Consumes n bytes from the beginning, moving the Buffer's beginning.
     * These bytes are considered garbage, which means, the caller can NOT rely on their existence in memory anymore.
     * They might be overwritten by any buffer re-arranging operation.
     * @param n The number of bytes to consume
     */
    virtual void consume(uint32_t n);

    /**
     * Reduces consumed bytes count by offsetDiff, increases count of used bytes by offsetDiff.
     * @param offsetDiff The offset difference in bytes
     */
    void reset(uint32_t offsetDiff = 0);

    // increase buffer size
    /**
     * Increases buffer capacity to newCapacity. Does nothing if buffer has this capacity already.
     * @param newCapacity New capacity in bytes
     * @return New capacity
     */
    const uint32_t increase(const uint32_t newCapacity);
    /**
     * Overloaded variant of increase(const uint32_t newCapacity) which initializes newly allocated memory to value.
     * @param value Byte value of newly allocated (free) memory
     * @return New capacity
     */
    const uint32_t increase(const uint32_t newCapacity, const uint8_t value);

    /**
     * Adds padded bytes with specified value to the Buffer, so that Buffer is newSize long. Padded bytes are marked as used.
     * @param value Byte value of padded bytes
     */
    void padd(const uint32_t newSize, const uint8_t value);

    /**
     * Returns the Buffer's size.
     */
    const uint32_t size() const;

    /**
     * Returns a direct (mutable) data pointer to the beginning (+ p) of Buffer's memory.
     * BE CAREFUL when reading/writing from/to this pointer because the CALLER has to ensure, no memory region outside
     * the Buffer's range is accessed!
     * @param p Start at p-th byte. Defaults to 0.
     */
    void *data(uint32_t p = 0);

    /**
     * Returns a constant (immutable) data pointer to the beginning (+ p) of Buffer's memory.
     * BE CAREFUL when reading from this pointer because the CALLER has to ensure, no memory region outside the Buffer's
     * range is accessed!
     * @param p Start at p-th byte. Defaults to 0.
     */
    const void *const_data(uint32_t p = 0) const;

    /**
     * Returns a BufferRange object from offset with size.
     *
     * WARNING: This function does not check if the specified range exists!
     * @param offset The byte to start the BufferRange from
     * @param size BufferRange's size
     */
    const BufferRange const_data(uint32_t offset, uint32_t size) const;

    /**
     * Marks n bytes used. This increases Buffer's size.
     * @param n Number of bytes
     */
    virtual void use(uint32_t n);

    /**
     * Clears the buffer. This resets used and consumed bytes counts.
     */
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
