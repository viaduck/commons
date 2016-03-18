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
    bool get(const String &key, std::function<bool(String &value)> callback);

    bool get(const String &key, std::function<bool(const String &)> callback) const;

    void set(String key, String value);

    void serialize(Buffer &out) const;

    bool deserialize(BufferRangeConst in);

private:
    std::unordered_multimap<const String, String> mInternal;
    std::set<String> mKeys;
};

#endif //CORE_KEYVALUESTORAGE_H

