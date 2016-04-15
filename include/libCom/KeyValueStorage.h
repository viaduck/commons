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

/**
 * Class storing Key-Value pairs of libCom Strings with the ability to serialize and deserialize it
 */
class KeyValueStorage : public Serializable {

    using Type = std::unordered_multimap<const String, Buffer>;
    enum class IterStatus {
        Break,
        Continue,
        Error
    };

    // helper for template instantiation which indicates deriving from Serializable
    template<bool>
    struct is_serializable { };

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
    template<typename T>
    bool get(const String &key, std::function<bool(const T &)> callback) const {
        return get(key, callback, is_serializable<std::is_base_of<Serializable, T>::value>());
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
    template<typename T>
    bool get(const String &key, std::function<bool(T &)> callback) {
        return get(key, callback, is_serializable<std::is_base_of<Serializable, T>::value>());
    }

    /**
     * Gets a key associated with only one value with a fallback option.
     *
     * The returned pointer is only valid until the next KeyValueStorage non-const method is called.
     *
     * @param T Value type
     * @param key The key to look up
     * @param fallback Value will be returned if key does not exist
     *
     * @return Immutable pointer to value (managed by KeyValueStorage) or nullptr (key does not exist and no fallback
     * or multiple values for key)
     */
    template<typename T>
    T *get(const String &key, T *fallback = nullptr) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            return fallback;
        else if (mInternal.count(key) == 1)
            return &mInternal.find(key)->second;            // return pointer to value

        return nullptr;     // multiple values for key
    }

    /**
     * Associates the value with the given key. If the key does not exist, it will be created.
     *
     * If unique is true, the key will only be associated with a single value (1:1 relation). This has the following
     * consequences:
     * - Key is already associated with >= 2 values -> Nothing will be done, false returned
     * - Key is associated with 1 value -> Key's value will be set to passed value
     * - Key isn't associated with any value yet -> Key's value will be set to passed value
     *
     * @param T Value type
     * @param key The key to associate the value with
     * @param value The value to associate
     * @param unique If true, 1:1 relation between key and value. Default: false
     * @return Setting has been successful
     */
    template<typename T>
    bool set(const String &key, const T &value, bool unique = false) {
        return set(is_serializable<std::is_base_of<Serializable, T>::value>(), key, value, unique);
    }

    /**
     * Reads a serialized KeyValueStorage from given Range.
     *
     * @param in Range pointing to serialized data
     *
     * @return True on success, false otherwise
     */
    bool deserialize(BufferRangeConst in);

    /**
     * Gets a key associated with only one value with a fallback option.
     *
     * The returned pointer is only valid until the next KeyValueStorage non-const method is called.
     *
     * @param T Value type
     * @param key The key to look up
     * @param fallback Value will be set to this and returned if key does not exist
     *
     * @return Mutable pointer to value (managed by KeyValueStorage) or nullptr (key does not exist and no fallback
     * or multiple values for key)
     */
    template<typename T>
    T *get(const String &key, T *fallback = nullptr) {
        // key does not exist
        if (mKeys.count(key) == 0) {
            // set value to fallback if specified
            if (fallback != nullptr) {
                set(key, *fallback);
                return get<T>(key);
            }
        } else if (mInternal.count(key) == 1) {
            return static_cast<T*>(mInternal.find(key)->second.data());        // return value casted to T
        }

        return nullptr;     // multiple values for key
    }

    /**
     * Serializes the KeyValueStorage into the given Buffer.
     *
     * @param out Buffer to append the serialized KeyValueStorage to
     */
    void serialize(Buffer &out) const;

    /**
     * Removes all items from this storage
     */
    inline void clear() {
        mKeys.clear();
        mInternal.clear();
    };

private:
    // internal key-value multimap
    Type mInternal;
    // internal key-set
    std::set<String> mKeys;

    // #### CONST ####
    template<typename T>
    bool get(const String &key, std::function<bool(const T &)> callback, is_serializable<true>) const {
        return iterEqualRange(key, [&](Type::const_iterator it) -> IterStatus {
            T t;
            if (!t.deserialize(it->second))
                return IterStatus::Error;       // error, cannot deserialize
            if (!callback(t))           // callback decided to break
                return IterStatus::Break;

            return IterStatus::Continue;
        });
    }

    template<typename T>
    bool get(const String &key, std::function<bool(const T &)> callback, is_serializable<false>) const {
        return iterEqualRange<Type::const_iterator>(key, [&](Type::const_iterator it) -> IterStatus {
            const Buffer &b = it->second;
            if (b.size() < sizeof(T))       // Buffer not big enough -> does not contain the requested value
                return IterStatus::Error;

            const T *data = static_cast<const T *>(b.const_data());
            if (!callback(ntoh(*data)))           // callback decided to break
                return IterStatus::Break;

            return IterStatus::Continue;
        });
    }

    // #### MUTABLE ####
    template<typename T>
    bool get(const String &key, std::function<bool(T &)> callback, is_serializable<false>) {
        return iterEqualRange<Type::iterator>(key, [&](Type::iterator it) -> IterStatus {
            Buffer &b = it->second;
            if (b.size() < sizeof(T))       // Buffer not big enough -> does not contain the requested value
                return IterStatus::Error;

            T data = ntoh(*static_cast<T *>(b.data()));
            if (!callback(data))           // callback decided to break
                return IterStatus::Break;

            // replace value since it could have been tainted by callback
            data = hton(data);
            b.write(&data, sizeof(T), 0);

            return IterStatus::Continue;
        });
    }
    template<typename T>
    bool get(const String &key, std::function<bool(T &)> callback, is_serializable<true>) {
        return iterEqualRange<Type::iterator>(key, [&](Type::iterator it) -> IterStatus {
            T t;
            if (!t.deserialize(it->second))
                return IterStatus::Error;       // error, cannot deserialize
            bool ret = callback(t);

            // replace value since it could have been tainted by callback
            t.serialize(it->second);

            if (!ret)         // callback decided to break
                return IterStatus::Break;

            return IterStatus::Continue;
        });
    }

    // #####################
    template<typename T>
    bool set(is_serializable<false>, const String &key, const T &value, bool unique = false) {
        return setInternal(key, unique, sizeof(T), [&](Buffer &b) {
                    T vvalue = hton(value);
                    b.append(&vvalue, sizeof(T));
                });
    }

    template<typename T>
    bool set(is_serializable<true>, const String &key, const T &value, bool unique = false) {
        // FIXME don't hardcode >>value.size()*2<<
        return setInternal(key, unique, value.size() * 2, [&](Buffer &b) {
                    value.serialize(b);
                });
    }

    /**
     * Helper function, that contains iteration logic (reduces code duplication)
     */
    template<typename IterType>
    bool iterEqualRange(const String &key, std::function<IterStatus(const IterType)> f) const {
        // range of values matching key
        auto range = mInternal.equal_range(key);

        // iterate range and call callback with every found value
        for (auto it = range.first; it != range.second; it++) {
            switch (f(it)) {
                case IterStatus::Break:
                    return true;
                case IterStatus::Continue:
                    continue;   // for loop
                case IterStatus::Error:
                    return false;    // error found, aborting
            }
        }

        return range.first != range.second;     // any value found for key
    }

    /**
     * Helper that supports mutable iterators
     */
    template<typename IterType>
    bool iterEqualRange(const String &key, std::function<IterStatus(IterType)> f) {
        // range of values matching key
        auto range = mInternal.equal_range(key);

        // iterate range and call callback with every found value
        for (auto it = range.first; it != range.second; it++) {
            switch (f(it)) {
                case IterStatus::Break:
                    return true;
                case IterStatus::Continue:
                    continue;   // for loop
                case IterStatus::Error:
                    return false;    // error found, aborting
            }
        }

        return range.first != range.second;     // any value found for key
    };

    /**
     * Helper for emplacing a value into map
     */
    bool setInternal(const String &key, bool unique, uint32_t expSize, std::function<void(Buffer &)> f) {
        if (unique) {
            size_t c = mInternal.count(key);
            if (c >= 2)
                return false;
            else if (c == 1)
                mInternal.erase(key);       // TODO inefficient
            // key does not exist -> will be inserted below
        }

        Buffer b(expSize);

        // serialize
        f(b);

        mInternal.emplace(key, b);
        mKeys.insert(key);
        return true;
    }
};


/**
 * Calls callback for every value belonging to given key. The Value will be immutable in the callback.
 *
 * Specialization for type Buffer
 *
 * @param key The key to look up
 * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
 *
 * @return True if key exists in Storage, false otherwise
 */
template <>
bool KeyValueStorage::get<Buffer>(const String &key, std::function<bool(const Buffer &)> callback) const;
template <>
bool KeyValueStorage::get<String>(const String &key, std::function<bool(const String &)> callback) const;

/**
 * Calls callback for every value belonging to given key. The Value will be mutable in the callback.
 *
 * Specialization for type Buffer.
 *
 * @param key The key to look up
 * @param callback A callback receiving every value of the key. Return false to terminate loop over values.
 *
 * @return True if key exists in Storage, false otherwise
 */
template <>
bool KeyValueStorage::get<Buffer>(const String &key, std::function<bool(Buffer &)> callback);
template <>
bool KeyValueStorage::get<String>(const String &key, std::function<bool(String &)> callback);

/**
 * Associates the value with the given key. If the key does not exist, it will be created.
 *
 * Specialization for type Buffer.
 *
 * @param key The key to associate the value with
 * @param value The value to associate
 */
template <>
bool KeyValueStorage::set<Buffer>(const String &key, const Buffer &value, bool unique);
template <>
bool KeyValueStorage::set<String>(const String &key, const String &value, bool unique);

template <>
Buffer * KeyValueStorage::get<Buffer>(const String &key, Buffer *fallback);
template <>
String * KeyValueStorage::get<String>(const String &key, String *fallback);

#endif //CORE_KEYVALUESTORAGE_H
