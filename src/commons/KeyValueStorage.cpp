#include <commons/KeyValueStorage.h>

bool KeyValueStorage::setInternal(const String &key, const Buffer &value, bool replaceExisting) {
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

void KeyValueStorage::modifyInternalMultiBuffer(const String &key, std::function<bool(Buffer &, bool &)> callback) {
    // range of values matching key
    auto range = mInternal.equal_range(key);

    // iterate range and call callback with every found value
    bool exit = false;
    for (auto it = range.first; it != range.second && !exit; ) {

        // call callback -> break if false
        if (!callback(it->second, exit))
            it = mInternal.erase(it);
        else
            it++;
    }
}

void KeyValueStorage::getInternalMultiBuffer(const String &key, std::function<bool(const Buffer &)> callback) const {
    // range of values matching key
    auto range = mInternal.equal_range(key);

    // iterate range and call callback with every found value
    for (auto it = range.first; it != range.second; it++) {

        // call callback -> break if false
        if (!callback(it->second))
            break;
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
            setInternal(key, value, false);
            in += value.size() + sizeof(uint32_t);
        }
    }

    return true;
}

bool KeyValueStorage::getBuffer(const String &key, Buffer &value, bool uniqueResult) const {
    // key does not exist or multiple values but requested unique result
    if (mKeys.count(key) == 0 || (mInternal.count(key) > 1 && uniqueResult))
        return false;

    value.clear();              // clear to prevent wrongly reported size
    value.write(mInternal.find(key)->second, 0);
    return true;
}

bool KeyValueStorage::getSetBuffer(const String &key, Buffer &value, const Buffer &fallback, bool uniqueResult) {
    // key does not exist
    if (mKeys.count(key) == 0)
        setBuffer(key, fallback);
    else if (uniqueResult && mInternal.count(key) > 1)
        return false;

    value.clear();              // clear to prevent wrongly reported size
    getBuffer(key, value, uniqueResult);
    return true;
}

void KeyValueStorage::serialize(Buffer &out) const {
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
