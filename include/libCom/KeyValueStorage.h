//
// Created by John Watson on 25.12.2016.
//

#ifndef LIBCOM_KEYVALUESTORAGENEW_H
#define LIBCOM_KEYVALUESTORAGENEW_H


#include <unordered_map>
#include <set>
#include "String.h"

/**
 * Class storing Key-Value pairs of libCom Strings with the ability to serialize and deserialize it.
 * Any type of value can be stored, including:
 * <ul>
 * <li>Value types such as int, double, float
 * <li>Serializable types such as String, Protocol classes, other KeyValueStorages
 * <li>Buffers, possibly containing any other type of data.
 */
class KeyValueStorage {
    using Type = std::unordered_multimap<const String, Buffer>;

public:
    /* KVS get methods, each with and without fallback */

    /**
     * Gets a Buffer from KVS.
     *
     * If multiple Buffers exist for key, any Buffer can be returned.
     * @return True on success, false if the key does not exist.
     */
    bool getBuffer(const String &key, Buffer &value) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            return false;

        value.clear();              // clear to prevent wrongly reported size
        value.write(mInternal.find(key)->second, 0);
        return true;
    }

    /**
     * Gets a Buffer from KVS.
     *
     * If multiple Buffers exist for key, any Buffer can be returned.
     * If the key does not exist, fallback will be returned and inserted into the KVS along with the key.
     */
    bool getSetBuffer(const String &key, Buffer &value, const Buffer &fallback) {
        // key does not exist
        if (mKeys.count(key) == 0)
            setBuffer(key, fallback);

        value.clear();              // clear to prevent wrongly reported size
        return getBuffer(key, value);
    }

    /**
     * Gets a value type T from KVS.
     *
     * If multiple values exist for key, any value can be returned.
     * If the key does not exist std::invalid_argument will be thrown.
     */
    template <typename T>
    T getValue(const String &key) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            throw std::invalid_argument("Key does not exist");

        return buffertoValue<T>(mInternal.find(key)->second);
    }

    /**
     * Gets a value type T from KVS.
     *
     * If multiple values exist for key, any value can be returned.
     * If the key does not exist, fallback will be returned and inserted into the KVS along with the key.
     */
    template <typename T>
    T getSetValue(const String &key, const T &fallback) {
        // key does not exist
        if (mKeys.count(key) == 0) {
            setValue(key, fallback);
            return fallback;
        }

        return buffertoValue<T>(mInternal.find(key)->second);
    }

    /**
     * Gets a Serializable from KVS and deserializes it into value.
     *
     * If multiple Serializables exist for key, any Serializable can be returned.
     * @return True on success, false if the key does not exist.
     */
    template <typename T>
    bool getSerializable(const String &key, T &value) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            return false;

        return value.deserialize(mInternal.find(key)->second);
    }

    /**
     * Gets a Serializable from KVS and deserializes it into value.
     *
     * If multiple Serializables exist for key, any Serializable can be returned.
     * If the key does not exist, fallback will be returned and inserted into the KVS along with the key.
     */
    template <typename T>
    bool getSetSerializable(const String &key, T &value, const T &fallback) {
        // key does not exist
        if (mKeys.count(key) == 0)
            setSerializable<T>(key, fallback);

        return getSerializable<T>(key, value);
    }

    /* KVS set methods */

    /**
    * Inserts the Buffer along with the key into the KVS.
    *
    * If replaceExisting is false and the key already exists, it will become a multi-value key.
    * If replaceExisting is true any existing Buffer will be replaced.
    *
    * @return True on success, false if replaceExisting is true but the key is already a multi-value key.
    */
    bool setBuffer(const String &key, const Buffer &value, bool replaceExisting = false) {
        return setInternal(key, value, replaceExisting);
    }

   /**
    * Inserts the value along with the key into the KVS.
    *
    * If replaceExisting is false and the key already exists, it will become a multi-value key.
    * If replaceExisting is true any existing value will be replaced.
    *
    * @return True on success, false if replaceExisting is true but the key is already a multi-value key.
    */
    template <typename T>
    bool setValue(const String &key, const T &value, bool replaceExisting = false) {
        Buffer insertMe(sizeof(T));
        valueToBuffer(value, insertMe);

        return setInternal(key, insertMe, replaceExisting);
    }

    /**
    * Inserts the serialized Serializable along with the key into the KVS.
    *
    * If replaceExisting is false and the key already exists, it will become a multi-value key.
    * If replaceExisting is true any existing Serializable will be replaced.
    *
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
     * Gets all Buffers belonging to key from KVS.
     *
     * @param callback Callback called for each Buffer. The order of the Buffers is undefined.
     * Returning true from callback indicates that the iteration should continue. Returning false breaks from loop.
     *
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
     * Gets all values belonging to key from KVS.
     *
     * @param callback Callback called for each value. The order of the values is undefined.
     * Returning true from callback indicates that the iteration should continue. Returning false breaks from loop.
     *
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
     * Gets all Serializables belonging to key from KVS.
     *
     * @param callback Callback called for each Serializable. The order of the Serializables is undefined.
     * Returning true from callback indicates that the iteration should continue. Returning false breaks from loop.
     *
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
     * @param callback Callback called for each mutable Buffer. The order of the Buffers is undefined.
     * Returning true from callback indicates that the iteration should continue. Returning false breaks from loop.
     *
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
     * Gets all values belonging to key from KVS with the ability to modify them in callback.
     *
     * @param callback Callback called for each mutable value. The order of the values is undefined.
     * Returning true from callback indicates that the iteration should continue. Returning false breaks from loop.
     *
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
     * Gets all Serializables belonging to key from KVS with the ability to modify them in callback.
     *
     * @param callback Callback called for each mutable Serializable. The order of the Serializables is undefined.
     * Returning true from callback indicates that the iteration should continue. Returning false breaks from loop.
     *
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
     * @param out Buffer to append the serialized KeyValueStorage to
     */
    void serialize(Buffer &out) const {
        out.clear();              // clear to prevent wrongly reported size
        for(const String &key : mKeys) {
            key.serialize(out);

            // serialize number of values
            uint32_t keyCount = mInternal.count(key);
            out.append(&keyCount, sizeof(keyCount));

            getBuffers(key, [&out](const Buffer &val) -> bool {
                val.serialize(out);
                return true;
            });
        }
    }

    /**
     * Reads a serialized KeyValueStorage from given Range.
     *
     * @param in Range pointing to serialized data
     * @return True on success, false otherwise
     */
    bool deserialize(BufferRangeConst in) {
        // remove all items before deserialize
        clear();

        // while there is still data to read
        while(in.size() > 0) {
            // deserialize key first
            String key;
            if(!key.deserialize(in))
                return false;

            // move in pointer by key size and size of serialized key header
            in += key.size() + sizeof(uint32_t);
            // fail if not enough data left for count of children
            if(in.size() < sizeof(uint32_t))
                return false;

            // read child count
            uint32_t valCount = *static_cast<const uint32_t*>(in.const_data());
            in += sizeof(uint32_t);

            // for each child, deserialize its value
            for (uint32_t cV = 0; cV < valCount; ++cV) {
                Buffer value;
                if(!value.deserialize(in))
                    return false;

                // store value in KVS and move pointer
                setInternal(key, value, false);
                in += value.size() + sizeof(uint32_t);
            }
        }

        return true;
    }

    /**
     * Removes all items from this storage
     */
    inline void clear() {
        mKeys.clear();
        mInternal.clear();
    };
private:
    /**
     * Converts the value in network byte-order from Buffer to host byte-order of type T.
     *
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
     */
    template <typename T>
    void valueToBuffer(const T &value, Buffer &out) {
        T valueConv = hton(value);
        out.write(&valueConv, sizeof(T), 0);
    }

    /**
     *  Calls callback for each readonly Buffer associated to key.
     */
    void getInternalMultiBuffer(const String &key, std::function<bool(const Buffer &)> callback) const {
        // range of values matching key
        auto range = mInternal.equal_range(key);

        // iterate range and call callback with every found value
        for (auto it = range.first; it != range.second; it++) {

            // call callback -> break if false
            if (!callback(it->second))
                break;
        }
    }

    /**
     *  Calls callback for each mutable Buffer associated to key.
     */
    void modifyInternalMultiBuffer(const String &key, std::function<bool(Buffer &)> callback) {
        // range of values matching key
        auto range = mInternal.equal_range(key);

        // iterate range and call callback with every found value
        for (auto it = range.first; it != range.second; it++) {

            // call callback -> break if false
            if (!callback(it->second))
                break;
        }
    }

    /**
     * Associates the value to the key.
     *
     * If existing value should be replaced and:
     * <ul>
     * <li>no value exists for key -> new value will be associated
     * <li>one value exists for key -> it will be replaced by new value
     * <li>more than one value exists for key -> error
     * </ul>
     *
     * @param replaceExisting Indicates whether to try to replace an existing value with given value
     * @return True on success, false if replaceExisting is true and more than one value associated with key
     */
    bool setInternal(const String &key, const Buffer &value, bool replaceExisting) {
        auto count = mInternal.count(key);

        if (replaceExisting) {
            // more than one element present, don't know which to replace
            if (count > 1)
                return false;
            else if (count == 1)
                mInternal.erase(key);

            // at this point key does not exist anymore and can be safely added below
        }

        mKeys.insert(key);
        mInternal.emplace(key, value);
        return true;
    }

    // internal key-value multimap
    Type mInternal;
    // internal key-set
    std::set<String> mKeys;
};


#endif //LIBCOM_KEYVALUESTORAGENEW_H
