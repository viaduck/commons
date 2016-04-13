//
// Created by John Watson on 18.03.16.
//

#ifndef CORE_KEYVALUESTORAGE_H
#define CORE_KEYVALUESTORAGE_H

#include <unordered_map>
#include <set>
#include <algorithm>
#include <type_traits>

#include "String.h"

template<typename T>
class HasPolicy
{
    template <typename U, void (U::*)(Buffer)> struct Check;
    template <typename U> static char func(Check<U, &U::serialize> *);
    template <typename U> static int func(...);
public:
    typedef HasPolicy type;
    enum { value = sizeof(func<T>(0)) == sizeof(char) };
};

/**
 * Class storing Key-Value pairs of libCom Strings with the ability to serialize and deserialize it
 */
class KeyValueStorage {
public:

    /**
     * Calls callback for every value belonging to given key. The Value will be immutable in the callback.
     *
     * @param T Value type
     * @param key The key to look up
     * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
     *
     * @return True if key exists in Storage, false otherwise
     */
    template <typename T, typename std::enable_if<HasPolicy<T>::value>::type>
    bool get(const String &key, std::function<bool(const T &)> callback) const {
        // range of values matching key
        auto range = mInternal.equal_range(key);

        // iterate range and call callback with every found value
        for (auto it = range.first; it != range.second; it++) {
            // TODO deserialize
            Buffer t;
            if (!callback(t))           // callback decided to break
                break;
        }

        return range.first != range.second;     // any value found for key
    }

    /**
     * Calls callback for every value belonging to given key. The Value will be immutable in the callback.
     *
     * @param T Value type (derived from Buffer)
     * @param key The key to look up
     * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
     *
     * @return True if key exists in Storage, false otherwise
     */
    template <typename T, class Enable = void>
    bool get(const String &key, std::function<bool(const T &)> callback) const {
        // range of values matching key
        auto range = mInternal.equal_range(key);

        // iterate range and call callback with every found value
        for (auto it = range.first; it != range.second; it++) {
            if (!callback(it->second))           // callback decided to break
                break;
        }

        return range.first != range.second;     // any value found for key
    }

    /**
     * Calls callback for every value belonging to given key. The Value will be mutable in the callback.
     *
     * @param T Value type
     * @param key The key to look up
     * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
     *
     * @return True if key exists in Storage, false otherwise
     */
    /*template <typename T>
    bool get(const String &key, std::function<bool(T &)> callback) {
        std::function<bool(const T&)> l = [&] (const T &value) -> bool {
            // replace tainted value
            bool continueIter = callback(const_cast<T &>(value));
            // TODO replace tainted value. Need an iterator for this..

            return continueIter;
        };
        return const_cast<const KeyValueStorage *>(this)->get(key, l);
    }*/

    /**
     * Associates the value with the given key. If the key does not exist, it will be created.
     *
     * @param T Value type
     * @param key The key to associate the value with
     * @param value The value to associate
     */
    template <typename T, class Enable = void>
    void set(const String &key, const T& value) {
        // serialize to Buffer
        Buffer b;
        b.append(&value, sizeof(T));        // TODO Big-Little-Endian
        setInternal(key, b);
    }

    /**
     * Associates the value with the given key. If the key does not exist, it will be created.
     *
     * @param T Value type (derived from Buffer)
     * @param key The key to associate the value with
     * @param value The value to associate
     */
    template <typename T, typename std::enable_if<!std::is_base_of<Buffer, T>::value>::type>
    void set(const String &key, const T& value) {
        Buffer b(value.size()+sizeof(uint32_t));        // FIXME don't hardcode this
        value.serialize(b);
        setInternal(key, b);
    };

    // TODO private
    template <typename T>
    void setInternal(const String &key, const T &value) {
        mInternal.emplace(key, value);
        mKeys.insert(key);
    }

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
     * @param T Value type
     * @param key The key to look up
     * @param fallback Value will be set to this and returned if key does not exist
     *
     * @return Mutable pointer to value (managed by KeyValueStorage) or nullptr (key does not exist and no fallback
     * or multiple values for key)
     */
    template <typename T>
    T *getSingle(const String &key, T *fallback = nullptr) {
        // key does not exist
        if (mKeys.count(key) == 0) {
            // set value to fallback if specified
            if (fallback != nullptr) {
                set(key, *fallback);
                return fallback;
            }
        } else if (mKeys.count(key) == 1) {
            // return value
            return &mInternal.find(key)->second;            // TODO pointer to local object?
        } else          // multiple values for key
            return nullptr;
    }

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
    std::unordered_multimap<const String, Buffer> mInternal;
    // internal key-set
    std::set<String> mKeys;
};

#endif //CORE_KEYVALUESTORAGE_H
