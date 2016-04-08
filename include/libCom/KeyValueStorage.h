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
     * Calls callback for every value belonging to given key. The Value will be mutable in the callback.
     *
     * @param key The key to look up
     * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
     *
     * @return True if key exists in Storage, false otherwise
     */
    template <typename T>
    bool getSet(const String &key, std::function<bool(T &)> callback);

    /**
     * Calls callback for every value belonging to given key. The Value will be immutable in the callback.
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

    /**
     * Gets a key with only one value with a fallback option.
     *
     * @param key The key to look up
     * @param fallback If specified the fallback will be set as value and returned if key does not exist
     *
     * @return Mutable pointer to value or nullptr
     */
    template <typename T>
    const T *getSingle(const String &key, const T *fallback = nullptr);

    /**
     * Sets a value to a key with only one value, overwriting existing value if needed
     *
     * @param key The key to set value for
     * @param value The value to associate with key
     *
     * @return True on success
     */
    template <typename T>
    bool setSingle(const String &key, const T &value);

    /**
     * Removes all items from this storage
     */
    void clear();

private:
    // internal key-value multimap
    std::unordered_multimap<const String, String> mInternal;
    // internal key-set
    std::set<String> mKeys;
};

#endif //CORE_KEYVALUESTORAGE_H

