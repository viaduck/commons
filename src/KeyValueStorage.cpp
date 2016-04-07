//
// Created by John Watson on 18.03.16.
//

#include <libCom/KeyValueStorage.h>

template <>
bool KeyValueStorage::getSet<String>(const String &key, std::function<bool(String &value)> callback) {
    // get all values matching key
    auto its = mInternal.equal_range(key);

    // call callback with every value
    for(auto it = its.first; it != its.second; it++) {
        // callback decided to break
        if(!callback(it->second))
            break;
    }

    return its.first != its.second;
}

// TODO: code duplicate for const version
template <>
bool KeyValueStorage::get<String>(const String &key, std::function<bool(const String &value)> callback) const {
    // get all values matching key
    auto its = mInternal.equal_range(key);

    // call callback with every value
    for(auto it = its.first; it != its.second; it++) {
        // callback decided to break
        if(!callback(it->second))
            break;
    }

    return its.first != its.second;
}

template <typename T>
bool KeyValueStorage::getSet(const String &key, std::function<bool(T &value)> callback) {
    return getSet<String>(key, [&](String &v) {
        T conv = *static_cast<const T*>(v.toBuffer().const_data());
        bool res = callback(conv);
        v = String(reinterpret_cast<uint8_t*>(&conv), sizeof(T));
        return res;
    });
}

template <typename T>
bool KeyValueStorage::get(const String &key, std::function<bool(const T &value)> callback) const {
    return get<String>(key, [&](const String &v) {
        T conv = *static_cast<const T*>(v.toBuffer().const_data());
        return callback(conv);
    });
}

template <>
void KeyValueStorage::set(const String &key, const String &value) {
    mInternal.emplace(key, value);
    mKeys.insert(key);
}

template <typename T>
void KeyValueStorage::set(const String &key, const T& value) {
    String s(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
    set(key, s);
}

void KeyValueStorage::serialize(Buffer &out) const {
    for(const String &key : mKeys) {
        key.serialize(out);

        // serialize number of values
        uint32_t keyCount = mInternal.count(key);
        out.append(&keyCount, sizeof(keyCount));

        get<String>(key, [&out](const String &val) -> bool {
            val.serialize(out);
            return true;
        });
    }
}


bool KeyValueStorage::deserialize(BufferRangeConst in) {
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
        if(in.size() < 4)
            return false;

        // read child count
        uint32_t valCount = *static_cast<const uint32_t*>(in.const_data());
        in += sizeof(uint32_t);

        // for each child, deserialize its value
        for (uint32_t cV = 0; cV < valCount; ++cV) {
            String value;
            if(!value.deserialize(in))
                return false;

            // store value in KVS and move pointer
            set(key, value);
            in += key.size() + sizeof(uint32_t);
        }
    }

    return true;
}

template<typename T>
T *KeyValueStorage::getSingle(const String &key, const T *fallback) {
    // get all values matching key
    auto its = mInternal.equal_range(key);

    // range is empty and fallback given
    if(its.first == its.second && fallback != nullptr) {
        set(key, *fallback);
        return getSingle<T>(key, nullptr);
    }
    else if(its.first != its.second)
        return &its.first->second;

    // range is empty and no fallback given
    return nullptr;
}

template<typename T>
bool KeyValueStorage::setSingle(const String &key, const T &value) {
    if(mKeys.count(key) > 0)
        return getSet<T>(key, [&] (T &listValue) {
            listValue = value;
            return false;
        });
    else
        set(key, value);

    return true;
}

void KeyValueStorage::clear() {
    mKeys.clear();
    mInternal.clear();
}


// explicit template instantiation
template bool KeyValueStorage::getSet<String>(const String &key, std::function<bool(String &value)> callback);
template bool KeyValueStorage::get<String>(const String &key, std::function<bool(const String &value)> callback) const;
template void KeyValueStorage::set<String>(const String &key, const String& value);
template String *KeyValueStorage::getSingle(const String &key, const String *fallback);

template bool KeyValueStorage::getSet<uint32_t>(const String &key, std::function<bool(uint32_t &value)> callback);
template bool KeyValueStorage::get<uint32_t>(const String &key, std::function<bool(const uint32_t &value)> callback) const;
template void KeyValueStorage::set<uint32_t>(const String &key, const uint32_t& value);
template bool KeyValueStorage::setSingle(const String &key, const String &value);