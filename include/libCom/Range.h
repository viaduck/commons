#ifndef LIBCOM_RANGE_H
#define LIBCOM_RANGE_H

#include "helper.h"

template <typename T>
class Range {

public:
    /**
     * Creates a Range (offset, size) within an object.
     * @param obj T
     * @param offset Offset in Object to start at
     * @param size Range's size
     */
    Range(T &obj, uint32_t offset, uint32_t size) : mObj(obj), mSize(size), mOffset(offset) { }

    /**
     * Overload constructor with offset = 0 and size = obj.size()
     * @param obj T
     */
    Range(T &obj) : Range(obj, 0, obj.size()) { }

    /**
     * Compares two Ranges
     * @param other
     * @return True if content within the ranges is the same
     */
    bool operator==(const Range &other) const {
        if (other.mSize != mSize)       // size is different -> they are truly not equal
            return false;

        const char *cthis = static_cast<const char *>(mObj.const_data(mOffset)),
                *cother = static_cast<const char *>(other.mObj.const_data(other.mOffset));

        return comparisonHelper(cthis, cother, mSize);
    }


    /**
     * Getter: const T
     * @return object
     */
    inline T &const_object() const {
        return mObj;
    }
    /**
     * Getter: T
     * @return object
     */
    inline T &object() {
        return mObj;
    }

    /**
     * Getter: size
     * @return size
     */
    inline const uint32_t size() const {
        return mSize;
    }
    /**
     * Setter: size
     */
    inline void size(uint32_t size) {
        mSize = size;
    }

    /**
     * Getter: offset
     * @return offset
     */
    inline const uint32_t offset() const {
        return mOffset;
    }
    /**
     * Setter: offset
     */
    inline void offset(uint32_t offset) {
        mOffset = offset;
    }

    /**
     * Getter: const underlying data (of T)
     */
    inline const void *const_data() const {
        return const_object().const_data(offset());
    }
    /**
     * Getter: underlying data (of T)
     */
    inline void *data() {
        return object().data(offset());
    }

    /**
     * Shrinks the Range by moving it forward.
     * Internally, this increases the offset and reduces the size.
     *
     * This operation is in-place and modifies the Range!
     * @param addition Amount of bytes to move
     * @return this
     */
    Range &operator+=(uint32_t addition) {
        if (addition > mSize)
            addition = mSize;

        mOffset += addition;
        mSize -= addition;
        return *this;
    }

private:
    T &mObj;
    uint32_t mSize;
    uint32_t mOffset;
};

#endif //LIBCOM_RANGE_H
