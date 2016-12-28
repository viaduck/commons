//
// Created by John Watson on 25.12.2016.
//

#ifndef LIBCOM_KEYVALUESTORAGE_H
#define LIBCOM_KEYVALUESTORAGE_H


#include <unordered_map>
#include <set>
#include "String.h"

/**
 * Class storing Key-Value pairs of libCom Strings with the ability to serialize and deserialize them.
 * Any type of value can be stored, including:
 * <ul>
 * <li>Value types such as int, double, float</li>
 * <li>Serializable types such as String, Protocol classes, other KeyValueStorages</li>
 * <li>Buffers, possibly containing any other type of data.</li>
 * </ul>
 */
class KeyValueStorage {
    using Type = std::unordered_multimap<const String, Buffer>;

public:
    /* KVS get methods, each with and without fallback */

    /**
     * Gets a Buffer from KVS.
     *
     * @param key Buffer's key.
     * @param value Buffer that will receive the key's value. It is overwritten.
     * @param uniqueResult If this is set to false and multiple Buffers exist for a key, any Buffer can be returned.
     * @return True on success, false if the key does not exist or there are multiple values but uniqueResult is true.
     */
    bool getBuffer(const String &key, Buffer &value, bool uniqueResult = true) const;

    /**
     * Gets a Buffer from KVS.
     *
     * If the key does not exist, fallback will be returned and inserted into the KVS at the given the key.
     * @param key Buffer's key
     * @param value Buffer that will receive the key's value. It is overwritten.
     * @param fallback Fallback for key if it does not exist.
     * @param uniqueResult If this is set to false and multiple Buffers exist for a key, any Buffer can be returned.
     * @throw std::invalid_argument if uniqueResult is true but there are multiple values for the key.
     */
    void getSetBuffer(const String &key, Buffer &value, const Buffer &fallback, bool uniqueResult = true);

    /**
     * Gets a value type T from KVS.
     *
     * @tparam T Type of value.
     * @param key Key to lookup
     * @param uniqueResult If this is set to false and multiple values exist for key, any value can be returned.
     * @throw std::invalid_argument if key does not exist or there are multiple values and uniqueResult is true.
     * @return Value associated with key.
     */
    template <typename T>
    T getValue(const String &key, bool uniqueResult = true) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            throw std::invalid_argument("Key does not exist");
        else if (mInternal.count(key) > 1 && uniqueResult)
            throw std::invalid_argument("Multiple values associated with key");

        return buffertoValue<T>(mInternal.find(key)->second);
    }

    /**
     * Gets a value type T from KVS.
     *
     * If the key does not exist, fallback will be returned and inserted into the KVS at the given key.
     * @tparam T Type of value.
     * @param key Key to lookup.
     * @param fallback Fallback for key's value.
     * @param uniqueResult If this is set to false and multiple values exist for a key, any value can be returned.
     * @throw std::invalid_argument if uniqueResult is true but there are multiple values for the key.
     * @return Value associated with key.
     */
    template <typename T>
    T getSetValue(const String &key, const T &fallback, bool uniqueResult = true) {
        // key does not exist
        if (mKeys.count(key) == 0) {
            setValue(key, fallback);
            return fallback;
        }
        else if (mInternal.count(key) > 1 && uniqueResult)
            throw std::invalid_argument("Multiple values associated with key");

        return buffertoValue<T>(mInternal.find(key)->second);
    }

    /**
     * Gets a Serializable from KVS and deserializes it into value.
     *
     * @tparam T Type of value.
     * @param key Key to lookup.
     * @param value Reference to object that will receive the value.
     * @param uniqueResult If this is set to false and multiple Serializables exist for a key, any Serializable can be
     *        returned.
     * @return True on success, false if the key does not exist or uniqueResult is true and there are multiple
     *         Serializables for the key.
     */
    template <typename T>
    bool getSerializable(const String &key, T &value, bool uniqueResult = true) const {
        // key does not exist or multiple values associated with key but unique result requested
        if (mKeys.count(key) == 0 || (mInternal.count(key) > 1 && uniqueResult))
            return false;

        return value.deserialize(mInternal.find(key)->second);
    }

    /**
     * Gets a Serializable from KVS and deserializes it into value.
     *
     * If the key does not exist, fallback will be returned and inserted into the KVS along with the key.
     * @tparam T Type of value.
     * @param key Key to lookup.
     * @param value Reference to object that will receive the value.
     * @param fallback Fallback value.
     * @param uniqueResult If this is set to false and multiple Serializables exist for a key, any Serializable can be
     *        returned.
     * @return True on success, false if the key does not exist or uniqueResult is true and there are multiple
     *         Serializables for the key.
     */
    template <typename T>
    bool getSetSerializable(const String &key, T &value, const T &fallback, bool uniqueResult = true) {
        // key does not exist
        if (mKeys.count(key) == 0)
            setSerializable<T>(key, fallback);
        else if (uniqueResult && mInternal.count(key) > 1)
            return false;

        return getSerializable<T>(key, value, uniqueResult);
    }

    /* KVS set methods */

    /**
     * Inserts the Buffer associated with the key into the KVS.
     *
     * @param key Associated key.
     * @param value Value to set the key to.
     * @param replaceExisting If false and key already esists, it will become a multi-value key. If true any existing
     *        key will be replaced
     * @return True on success, false if replaceExisting is true but the key is already a multi-value key.
     */
    bool setBuffer(const String &key, const Buffer &value, bool replaceExisting = false) {
        return setInternal(key, value, replaceExisting);
    }

    /**
     * Inserts the value associated with the key into the KVS.
     *
     * @tparam T Type of value.
     * @param key Associated key.
     * @param value Value to set the key to.
     * @param replaceExisting If false and key already esists, it will become a multi-value key. If true any existing
     *        key will be replaced
     * @return True on success, false if replaceExisting is true but the key is already a multi-value key.
     */
    template <typename T>
    bool setValue(const String &key, const T &value, bool replaceExisting = false) {
        Buffer insertMe(sizeof(T));
        valueToBuffer(value, insertMe);

        return setInternal(key, insertMe, replaceExisting);
    }

    /**
     * Inserts the serialized Serializable associated with the key into the KVS.
     *
     * @param key Associated key.
     * @param value Value to set the key to.
     * @param replaceExisting If false and key already esists, it will become a multi-value key. If true any existing
     *        key will be replaced
     * @return True on success, false if replaceExisting is true but the key is already a multi-value key.
     */
    template <typename T>
    bool setSerializable(const String &key, const T &value, bool replaceExisting = false) {
        Buffer insertMe(value.size());

        value.serialize(insertMe);
        return setInternal(key, insertMe, replaceExisting);
    }

    /* KVS multi get methods */

    /**
     * Gets all Buffers associated with the key from KVS.
     *
     * @param key Key to lookup.
     * @param callback Callback called for each Buffer. The order of the Buffers is undefined.
     *        Returning true from callback indicates that the iteration should continue. Returning false breaks loop.
     * @return True on success, false if key does not exist.
     */
    bool getBuffers(const String &key, std::function<bool(const Buffer &)> callback) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            return false;

        getInternalMultiBuffer(key, callback);
        return true;
    }

    /**
     * Gets all values associated with the key from KVS.
     *
     * @tparam T Type of value.
     * @param key Key to lookup.
     * @param callback Callback called for each value. The order of the values is undefined.
     *        Returning true from callback indicates that the iteration should continue. Returning false breaks loop.
     * @return True on success, false if key does not exist.
     */
    template <typename T>
    bool getValues(const String &key, std::function<bool(const T &)> callback) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            return false;

        return getBuffers(key, [&] (const Buffer &value) {
            T temporary = buffertoValue<T>(value);
            return callback(temporary);
        });
    }

    /**
     * Gets all Serializables associated with the key from KVS.
     *
     * @tparam T Type of value.
     * @param key Key to lookup.
     * @param callback Callback called for each Serializable. The order of the Serializables is undefined.
     *        Returning true from callback indicates that the iteration should continue. Returning false breaks loop.
     * @return True on success, false if key does not exist.
     */
    template <typename T>
    bool getSerializables(const String &key, std::function<bool(const T &)> callback) const {
        T temporary;

        return getBuffers(key, [&] (const Buffer &value) {
            if (!temporary.deserialize(value))
                throw std::invalid_argument("Malformed Serializable");

            return callback(temporary);
        });
    }

    /* KVS modify methods */

    /**
     * Gets all Buffers belonging to key from KVS with the ability to modify them in callback.
     *
     * @param key Key to lookup.
     * @param callback Callback called for each mutable Buffer. The order of the Buffers is undefined.
     *        Returning true from callback indicates that the iteration should continue. Returning false breaks loop.
     * @return True on success, false if key does not exist.
     */
    bool modifyBuffers(const String &key, std::function<bool(Buffer &)> callback) {
        // key does not exist
        if (mKeys.count(key) == 0)
            return false;

        modifyInternalMultiBuffer(key, callback);
        return true;
    }

    /**
     * Gets all values associated with the key from KVS. The callback has the ability to modify them inplace.
     *
     * @tparam T Type of value.
     * @param key Key to lookup.
     * @param callback Callback called for each mutable value. The order of the values is undefined.
     *        Returning true from callback indicates that the iteration should continue. Returning false breaks loop.
     * @return True on success, false if key does not exist.
     */
    template <typename T>
    bool modifyValues(const String &key, std::function<bool(T &)> callback) {
        // key does not exist
        if (mKeys.count(key) == 0)
            return false;

        return modifyBuffers(key, [&] (Buffer &value) {
            T temporary = buffertoValue<T>(value);

            bool shouldContinue = callback(temporary);
            valueToBuffer(temporary, value);

            return shouldContinue;
        });
    }

    /**
     * Gets all Serializables associated with the key from KVS. The callback has the ability to modify them inplace.
     *
     * @tparam T Type of value.
     * @param key Key to lookup.
     * @param callback Callback called for each mutable Serializable. The order of the Serializables is undefined.
     *        Returning true from callback indicates that the iteration should continue. Returning false breaks loop.
     * @return True on success, false if key does not exist.
     */
    template <typename T>
    bool modifySerializables(const String &key, std::function<bool(T &)> callback) {
        T temporary;

        return modifyBuffers(key, [&] (Buffer &value) {
            if (!temporary.deserialize(value))
                throw std::invalid_argument("Malformed Serializable");

            bool shouldContinue = callback(temporary);

            value.clear();
            temporary.serialize(value);

            return shouldContinue;
        });
    }

    /* KVS general management methods */

    /**
     * Serializes the KeyValueStorage into the given Buffer.
     *
     * @param out Buffer the serialized KeyValueStorage will be written to. It is cleared.
     */
    void serialize(Buffer &out) const;

    /**
     * Deserializes a former serialized KeyValueStorage from given Range.
     *
     * The KVS is cleared beforehand.
     *
     * @param in Range pointing to serialized data.
     * @return True on success, false otherwise.
     */
    bool deserialize(BufferRangeConst in);

    /**
     * Removes all items from this storage
     */
    inline void clear() {
        mKeys.clear();
        mInternal.clear();
    }
private:
    /**
     * Converts the value in network byte-order from Buffer to host byte-order of type T.
     *
     * @tparam Value type.
     * @param in Source Buffer.
     * @return Converted value.
     * @throws std::out_of_range if Buffer is too small
     */
    template <typename T>
    T buffertoValue(const Buffer &in) const {
        if (in.size() < sizeof(T))
            throw std::out_of_range("Buffer too small for Type");

        return ntoh(*static_cast<const T*>(in.const_data()));
    }

    /**
     * Converts the value in host byte-order of type T to network byte-order in Buffer.
     * @tparam Value type.
     * @param value Source value.
     * @param out Destination Buffer.
     */
    template <typename T>
    void valueToBuffer(const T &value, Buffer &out) {
        T valueConv = hton(value);
        out.write(&valueConv, sizeof(T), 0);
    }

    /**
     * Calls callback for each readonly Buffer associated to key.
     * @param key Key to lookup.
     * @callback If callback returns false, the lookup will be aborted.
     */
    void getInternalMultiBuffer(const String &key, std::function<bool(const Buffer &)> callback) const;

    /**
     * Calls callback for each mutable Buffer associated to key.
     * @param key Key to lookup.
     * @callback If callback returns false, the lookup will be aborted.
     */
    void modifyInternalMultiBuffer(const String &key, std::function<bool(Buffer &)> callback);

    /**
     * Associates the value with the key.
     *
     * If existing value should be replaced and:
     * <ul>
     * <li>no value exists for key -> new value will be associated</li>
     * <li>one value exists for key -> it will be replaced by new value</li>
     * <li>more than one value exists for key -> error</li>
     * </ul>
     *
     * @param key Associated key.
     * @param value Value to set the key to.
     * @param replaceExisting Indicates whether to try to replace an existing value with given value.
     * @return True on success, false if replaceExisting is true and more than one value associated with key.
     */
    bool setInternal(const String &key, const Buffer &value, bool replaceExisting);

    // internal key-value multimap
    Type mInternal;
    // internal key-set
    std::set<String> mKeys;
};

#endif //LIBCOM_KEYVALUESTORAGE_H
