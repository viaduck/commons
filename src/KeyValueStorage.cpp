//
// Created by John Watson on 18.03.16.
//

#include <algorithm>
#include <set>
#include <unordered_map>
#include <libCom/KeyValueStorage.h>

void KeyValueStorage::serialize(Buffer &out) const {
    for(const String &key : mKeys) {
        key.serialize(out);

        // serialize number of values
        uint32_t keyCount = mInternal.count(key);
        out.append(&keyCount, sizeof(keyCount));

        get<Buffer>(key, [&out](const Buffer &val) -> bool {
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
            set(key, value);
            in += value.size() + sizeof(uint32_t);
        }
    }

    return true;
}

template <>
bool KeyValueStorage::get<Buffer>(const String &key, std::function<bool(const Buffer &)> callback) const {
    return iterEqualRange<Type::const_iterator>(key, [&](Type::const_iterator it) -> IterStatus {
        if (!callback(it->second))           // callback decided to break
            return IterStatus::BREAK;

        return IterStatus::CONTINUE;
    });
}

template <>
bool KeyValueStorage::get<Buffer>(const String &key, std::function<bool(Buffer &)> callback) {
    return iterEqualRange<Type::iterator>(key, [&](Type::iterator it) -> IterStatus {
        if (!callback(it->second))           // callback decided to break
            return IterStatus::BREAK;

        // tainted value will be reflected automatically in multimap since callback takes a reference

        return IterStatus::CONTINUE;
    });
}

template <>
bool KeyValueStorage::set(const String &key, const Buffer &value, bool unique) {
    return setInternal(key, unique, value.size(), [&](Buffer &b) {
            b.append(value);
        });
}
