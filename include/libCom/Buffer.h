#ifndef LIBCOM_BUFFER_H
#define LIBCOM_BUFFER_H

#include <libCom/Serializable.h>
#include <libCom/SecureUniquePtr.h>
#include <cinttypes>
#include <libCom/Range.h>

class Buffer;

typedef Range<Buffer> BufferRange;
typedef Range<const Buffer> BufferRangeConst;

class DevNull;

class Buffer : public Serializable{
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
    virtual ~Buffer();

    /**
     * Appends a bunch of data to the Buffer; increases it's capacity if necessary.
     *
     * Capacity is at least double the capacity before.
     * @param data Data pointer
     * @param len Length of data (in bytes)
     * @return Range containing information about added range within Buffer
     */
    virtual BufferRangeConst append(const void *data, uint32_t len);
    /**
     * Overloaded variant of append(const void *data, uint32_t len) which accepts char* for convenience.
     * @param data Data pointer
     * @param len Length of data (in bytes)
     * @return Range containing information about added range within Buffer
     */
    virtual BufferRangeConst append(const char *data, uint32_t len);
    /**
     * Overloaded variant of append(const void *dta, uint32_t len) which appends the contents of another buffer to this
     * buffer.
     * @param other Other Buffer
     * @return Range containing information about added range within Buffer
     */
    virtual BufferRangeConst append(const Buffer &other);
    /**
     * Overloaded variant of append(const void *dta, uint32_t len) which appends the contents of another buffer described
     * by a Range to this buffer.
     * @param other Other Buffer
     * @return Range containing information about added range within Buffer
     */
    virtual BufferRangeConst append(const BufferRangeConst &range);

    /**
     * Writes a bunch of data to the Buffer starting at specified offset; overwriting existing data and increasing
     * Buffer's capacity if necessary.
     * @param data Data pointer (if nullptr: only allocate (if neceassary) and use memory)
     * @param len Length of data (in bytes)
     * @param offset Starting position
     * @return Range containing information about added range within Buffer
     */
    virtual BufferRangeConst write(const void *data, uint32_t len, uint32_t offset);
    /**
     * Overloaded variant of write(const void *data, uint32_t len, uint32_t offset) which writes the contents of another
     * buffer to this buffer.
     * @param other Other Buffer
     * @param offset Starting position
     * @return Range containing information about added range within Buffer
     */
    virtual BufferRangeConst write(const Buffer &other, uint32_t offset);
    /**
     * Overloaded variant of write(const void *data, uint32_t len, uint32_t offset) which writes the contents of another
     * buffer described by a Range to this buffer.
     * @param other Other Buffer
     * @param offset Starting position
     * @return Range containing information about added range within Buffer
     */
    virtual BufferRangeConst write(const BufferRangeConst &other, uint32_t offset);

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

    /**
     * Increases buffer capacity to newCapacity. Does nothing if buffer has this capacity already.
     * @param newCapacity New capacity in bytes
     * @param by True if newCapacity is relative to size() or false if not (standard)
     * @return New capacity
     */
    uint32_t increase(const uint32_t newCapacity, const bool by = false);
    /**
     * Overloaded variant of increase(const uint32_t newCapacity) which initializes newly allocated memory to value.
     * @param value Byte value of newly allocated (free) memory
     * @param by True if newCapacity is relative to size() or false if not (standard)
     * @return New capacity
     */
    uint32_t increase(const uint32_t newCapacity, const uint8_t value, const bool by = false);

    /**
     * Adds padded bytes with specified value to the Buffer (starting at offset), so that Buffer is newSize long. Padded
     * bytes are marked as used. Does not overwrite any existing byte.
     * @param offset Offset within Buffer
     * @param size Number of bytes to add
     * @param value Byte value of padded bytes
     */
    void padd(const uint32_t offset, const uint32_t size, const uint8_t value);

    /**
     * Overload variant of padd(const uint32_t offset, const uint32_t size, const uint8_t value) which starts at
     * end of Buffer (size())
     * @param newSize Buffer's new size
     * @param value Byte value of padded bytes
     */
    void padd(const uint32_t newSize, const uint8_t value);

    /**
     * Returns the Buffer's size.
     */
    virtual uint32_t size() const;

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
     * Returns a Range object from offset with size.
     *
     * WARNING: This function does not check if the specified range exists!
     * @param offset The byte to start the Range from
     * @param size Range's size
     */
    const BufferRangeConst const_data(uint32_t offset, uint32_t size) const;

    /**
     * Marks n bytes used. This increases Buffer's size.
     * @param n Number of bytes
     */
    virtual void use(uint32_t n);

    /**
     * Marks n bytes unused (resp. reverts use(..)). This decreases Buffer's size.
     * @param n Number of bytes
     */
    virtual void unuse(uint32_t n);

    /**
     * Clears the buffer. This resets used and consumed bytes counts.
     */
    virtual void clear();

    /**
     * Compares two Buffers.
     *
     * This is a deep comparison, comparing the used bytes only, not taking into account Buffer's capacity.
     * @param other
     * @return True if contents of Buffers are the same
     */
    virtual bool operator==(const Buffer &other) const;
    /**
     * Compares two Buffers.
     *
     * This is a deep comparison, comparing the used bytes only, not taking into account Buffer's capacity.
     * @param other
     * @return True if contents of Buffers differ
     */
    virtual inline bool operator!=(const Buffer &other) const {
        return !operator==(other);
    }

    /**
     * Serializes the Buffer to given Buffer.
     *
     * @param out Buffer where the serialized representation of this Buffer will be appended to
     */
    void serialize(Buffer &out) const {
        uint32_t cSize = size();
        // TODO Big-Little-Endian
        out.append(&cSize, sizeof(cSize));
        out.append(*this);
    }

    /**
     * Reads a serialized Buffer from given Range.
     *
     * @param in Range pointing to a serialized representation of a Buffer
     *
     * @return True on success, false otherwise
     */
    bool deserialize(BufferRangeConst in) {
        // TODO Big-Little-Endian
        if(in.size() < sizeof(uint32_t))
            return false;

        uint32_t cSize = *static_cast<const uint32_t*>(in.const_data());
        in += sizeof(uint32_t);

        if(in.size() < cSize)
            return false;

        append(in.const_data(), cSize);
        return true;
    }

private:
    SecureUniquePtr<uint8_t[]> mData;
    uint32_t mReserved;
    uint32_t mUsed = 0;
    uint32_t mOffset = 0;
};

// fix IDE quick fix tooltip, there is no other reason in doing this besides that
#include "DevNull.h"

#endif //LIBCOM_BUFFER_H
