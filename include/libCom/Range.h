#ifndef LIBCOM_RANGE_H
#define LIBCOM_RANGE_H

#include "helper.h"

/**
 * SFINAE test for checking the presence of a function padd
 * @tparam T Class, which is checked for member function presence
 */
template <typename T>
class has_padd
{
    typedef char one;
    typedef long two;

    static_assert(sizeof(char) != sizeof(long), "Unsupported platform: sizeof(char) == sizeof(long)");

    template <typename C> static one test(decltype(&C::padd));
    template <typename C> static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

template <typename T>
class Range {

public:
    /**
     * Creates a Range (offset, size) within an object.
     * @param obj T
     * @param offset Offset in Object to start at
     * @param size Range's size
     * @param resizable Defines if the Range is resizable
     */
    Range(T &obj, uint32_t offset, uint32_t size, bool resizable = false) :
            mObj(obj), mSize(size), mOffset(offset), mResizable(resizable) { }

    /**
     * Overload constructor with offset = 0 and size = obj.size()
     * @param obj T
     */
    Range(T &obj) : Range(obj, 0, obj.size(), true) { }

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
    inline uint32_t size() const {
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
    inline uint32_t offset() const {
        return mOffset;
    }
    /**
     * Setter: offset
     */
    inline void offset(uint32_t offset) {
        mOffset = offset;
    }

    /**
     * @return If the Range is resizable
     */
    inline bool isResizable() const {
        return mResizable;
    }
    /**
     * Sets the resizable property
     * @param resizable If the Range is resizable
     */
    inline void isResizable(bool resizable) {
        mResizable = resizable;
    }

    /**
     * Getter: const underlying data (of T)
     * @param pos Offset (defaults to 0)
     */
    inline const void *const_data(uint32_t pos = 0) const {
        return const_object().const_data(pos+offset());
    }
    /**
     * Getter: underlying data (of T)
     * @param pos Offset (defaults to 0)
     */
    inline void *data(uint32_t pos = 0) {
        return object().data(pos+offset());
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

    /**
     * Padds the Range to size (if possible).
     * @param dest Range to padd
     * @param size Target size
     * @return True if Range has the required size or can be increased to size
     */
    template<typename = std::enable_if<has_padd<T>::value, int>>
    static bool applyPolicy(Range<T> &dest, uint32_t size) {
        // not enough size and not resizable -> fail
        if (size > dest.size() && !dest.isResizable())
            return false;

        // not enough size and resizable -> increase size (will be applied to buffer in padd)
        else if (size > dest.size())
            dest.size(size);

        // the range in the buffer will be allocated and initialized to zero
        dest.object().padd(dest);
        return true;
    }

private:
    T &mObj;
    uint32_t mSize;
    uint32_t mOffset;
    bool mResizable = false;
};

#endif //LIBCOM_RANGE_H
