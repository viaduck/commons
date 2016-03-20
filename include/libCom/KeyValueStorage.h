//
// Created by John Watson on 18.03.16.
//

#ifndef CORE_KEYVALUESTORAGE_H
#define CORE_KEYVALUESTORAGE_H

#include <unordered_map>
#include <set>
#include <algorithm>
#include "String.h"

/**
 * Class storing Key-Value pairs of libCom Strings with the ability to serialize and deserialize it
 */
class KeyValueStorage {
public:

    /**
     * Calls callback for every value belonging to given key. The String will be mutable in the callback.
     *
     * @param key The key to look up
     * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
     *
     * @return True if key exists in Storage, false otherwise
     */
    template <typename T>
    bool getSet(const String &key, std::function<bool(T &)> callback);

    /**
     * Calls callback for every value belonging to given key. The String will be immutable in the callback.
     *
     * @param key The key to look up
     * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
     *
     * @return True if key exists in Storage, false otherwise
     */
    template <typename T>
    bool get(const String &key, std::function<bool(const T &)> callback) const;

    /**
     * Associates the value with the given key. If the key does not exist, it will be created.
     *
     * @param key The key to associate the value with
     * @param value The value to associate
     */
    template <typename T>
    void set(const String &key, const T& value);

    /**
     * Serializes the KeyValueStorage into the given Buffer.
     *
     * @param out Buffer to append the serialized KeyValueStorage to
     */
    void serialize(Buffer &out) const;

    /**
     * Reads a serialized KeyValueStorage from given Range.
     *
     * @param in Range pointing to serialized data
     *
     * @return True on success, false otherwise
     */
    bool deserialize(BufferRangeConst in);

private:
    // internal key-value multimap
    std::unordered_multimap<const String, String> mInternal;
    // internal key-set
    std::set<String> mKeys;
};

#endif //CORE_KEYVALUESTORAGE_H

