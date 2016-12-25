//
// Created by John Watson on 25.12.2016.
//

#ifndef LIBCOM_KEYVALUESTORAGENEW_H
#define LIBCOM_KEYVALUESTORAGENEW_H


#include <unordered_map>
#include <set>
#include "String.h"

class KeyValueStorageNew {
    using Type = std::unordered_multimap<const String, Buffer>;

public:
    /**  3 Single Value getters */

    template <typename T>
    T getValue(const String &key) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            throw std::invalid_argument("Key does not exist");

        return getInternal(key);
    }

    template <typename T>
    T getValue(const String &key, T fallback) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            return fallback;

        return getInternal(key);
    }

    template <typename T>
    T getSetValue(const String &key, const T &fallback) {
        // key does not exist
        if (mKeys.count(key) == 0) {
            setValue(key, fallback);
            return fallback;
        }

        return getInternal(key);
    }

    /** 1 Single Buffer getter */

    bool getBuffer(const String &key, Buffer &value) const {
        // key does not exist
        if (mKeys.count(key) == 0)
            return false;

        getInternal(key, value);
        return true;
    }

    /** 1 Single Serializable getter */

    template <typename T>
    bool getSerializable(const String &key, T &value) const {

        // get buffer first
        Buffer temporary;
        getInternal(key, temporary);

        return value.deserialize(temporary);
    }

    /** 1 Value setter */

    template <typename T>
    bool setValue(const String &key, const T &value, bool replaceExisting = false) {
        Buffer insertMe(sizeof(T));
        insertMe.write(&value, sizeof(T));

        return setInternal(key, insertMe, replaceExisting);
    }

    /** 1 Buffer setter */

    bool setBuffer(const String &key, const Buffer &value, bool replaceExisting = false) {
        return setInternal(key, value, replaceExisting);
    }

    /** 1 Serializable setter */

    template <typename T>
    bool setSerializable(const String &key, const T &value, bool replaceExisting = false) {
        Buffer insertMe(value.size());

        value.serialize(insertMe);
        return setInternal(key, insertMe, replaceExisting);
    }

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
    template <typename T>
    T getInternal(const String &key) const {
        return ntoh(*static_cast<T*>(mInternal.find(key)->second.const_data()));
    }

    void getInternal(const String& key, Buffer &value) const {
        value.write(mInternal.find(key)->second, 0);
    }

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
