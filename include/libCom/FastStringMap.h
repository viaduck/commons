//
// Created by John Watson on 02.11.16.
//

#ifndef CORE_FASTSTRINGMAP_H
#define CORE_FASTSTRINGMAP_H

#include <unordered_map>
#include "Buffer.h"
#include "String.h"

/**
 * Class providing fast Mapping for libCom String Keys
 * @param T Value type of the map
 */
template <typename T>
class FastStringMap {
public:
    /**
     * Adds an Element to the map
     *
     * @param key Key of the entry pair.
     * @param value Value to associate with the key. Existing keys' value will be overwritten
     */
    inline void add(const String &key, T value) {
        BufferRangeConst keyRange = mInternal.append(key);
        mMap[keyRange] = value;
        mReverseMap.emplace(value, std::move(keyRange));
    }

    /**
     * Looks the key up in the map
     *
     * @param key Key to look up
     * @param result Optional pointer to the lookup result. If not given, only a check for existence will be performed.
     * @return True if the lookup was successful and the element was found, false otherwise
     */
    inline bool lookup(const String &key, T *result = nullptr) const {
        BufferRangeConst stringRange = key.const_data(0, key.size());

        // does not exist
        if (mMap.find(stringRange) == mMap.end())
            return false;

        // result is required
        if (result != nullptr)
            *result = mMap.at(stringRange);

        return true;
    }

    /**
     * Removes an Element by value. Also removes the key associated to it.
     *
     * @param value The value to remove
     * @return True if the element was found, false otherwise
     */
    inline bool removeByVal(T value) {
        // check existence to avoid accidental inserting
        if (mReverseMap.find(value) == mReverseMap.end())
            return false;

        // find keyrange
        BufferRangeConst &keyRange = mReverseMap.at(value);

        // remove values
        mMap.erase(keyRange);
        mReverseMap.erase(value);
        return true;
    }

    /**
     * Clears and shreds all internal resources
     */
    inline void clear() {
        mInternal.clear(true);
        mMap.clear();
        mReverseMap.clear();
    }
private:
    // store all keys here
    Buffer mInternal;

    // mapping ranges in mInternal to value
    std::unordered_map<const BufferRangeConst, T> mMap;

    // mapping value to internal ranges
    std::unordered_map<T, BufferRangeConst> mReverseMap;
};

#endif //CORE_FASTSTRINGMAP_H
