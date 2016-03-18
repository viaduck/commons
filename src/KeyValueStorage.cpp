//
// Created by John Watson on 18.03.16.
//

#include <libCom/KeyValueStorage.h>

bool KeyValueStorage::get(const String &key, std::function<bool(String &value)> callback) {
    // get all values matching key
    auto its = mInternal.equal_range(key);

    // call callback with every value
    for(auto it = its.first; it != its.second; it++) {
        // callback decided to break
        if(!callback(it->second))
            break;
    }

    return its.first == its.second;
}

// TODO: code duplicate for const version
bool KeyValueStorage::get(const String &key, std::function<bool(const String &value)> callback) const {
    // get all values matching key
    auto its = mInternal.equal_range(key);

    // call callback with every value
    for(auto it = its.first; it != its.second; it++) {
        // callback decided to break
        if(!callback(it->second))
            break;
    }

    return its.first == its.second;
}

void KeyValueStorage::set(String key, String value) {
    mInternal.emplace(key, value);
    mKeys.insert(key);
}

void KeyValueStorage::serialize(Buffer &out) const {
    for(const String &key : mKeys) {
        key.serialize(out);

        // serialize number of values
        uint keyCount = mInternal.count(key);
        out.append(&keyCount, sizeof(keyCount));

        get(key, [&out](const String &val) -> bool {
            val.serialize(out);
            return true;
        });
    }
}


bool KeyValueStorage::deserialize(BufferRangeConst in) {
    while(in.size() > 0) {
        String key;
        if(!key.deserialize(in))
            return false;

        in += key.size() + sizeof(uint);
        if(in.size() < 4)
            return false;

        uint valCount = *static_cast<const uint*>(in.const_data());
        in += sizeof(uint);

        for (uint cV = 0; cV < valCount; ++cV) {
            String value;
            if(!value.deserialize(in))
                return false;

            set(key, value);
            in += key.size() + sizeof(uint);
        }
    }

    return true;
}