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
        mMap[mInternal.append(key)] = value;
    }

    /**
     * Looks the key up in the map
     *
     * @param key Key to look up
     * @param result Optional pointer to the lookup result. If not given, only a check for existence will be performed.
     * @return True if the lookup was successful and the element was found, false otherwise
     */
    inline bool lookup(const String &key, T *result = nullptr) {
        BufferRangeConst stringRange = key.const_data(0, key.size());

        // does not exist
        if (mMap.find(stringRange) == mMap.end())
            return false;

        // result is required
        if (result != nullptr)
            *result = mMap[stringRange];

        return true;
    }

    /**
     * Clears and shreds all internal resources
     */
    inline void clear() {
        mInternal.clear(true);
        mMap.clear();
    }
private:
    // store all keys here
    Buffer mInternal;

    // mapping ranges in mInternal to value
    std::unordered_map<const BufferRangeConst, T> mMap;
};

#endif //CORE_FASTSTRINGMAP_H
