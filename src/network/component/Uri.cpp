/*
 * Copyright (C) 2023 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <network/component/Uri.h>
#include <commons/util/Str.h>

#include <algorithm>
#include <iomanip>
#include <tuple>

static std::tuple<std::string, std::string> splitTwo(const std::string &str, const std::string &delim) {
    auto parts = Str::splitAll(str, delim, 2);

    auto part0 = parts.size() > 0 ? parts[0] : "";
    auto part1 = parts.size() > 1 ? parts[1] : "";
    return { part0, part1 };
}

static Uri::KeyValuePairList_t splitKeyValuePairs(const std::string &str) {
    Uri::KeyValuePairList_t result;

    auto parts = Str::splitAll(str, "&");
    for (const auto &part : parts) {
        auto kvp = Str::splitAll(part, "=", 2);

        auto kvp0 = kvp.size() > 0 ? Uri::decode(kvp[0]) : "";
        auto kvp1 = kvp.size() > 1 ? Uri::decode(kvp[1]) : "";

        if (!kvp0.empty())
            result.emplace_back(kvp0, kvp1);
    }

    return result;
}

static std::string joinKeyValuePairs(const Uri::KeyValuePairList_t &keyValuePairs) {
    std::vector<std::string> parts;

    for (const auto &keyValuePair : keyValuePairs) {
        if (keyValuePair.second.empty())
            parts.emplace_back(Uri::encode(keyValuePair.first));
        else
            parts.emplace_back(Uri::encode(keyValuePair.first) + "=" + Uri::encode(keyValuePair.second));
    }

    return Str::joinAll(parts, "&");
}

static std::optional<std::string> findValueByKey(const Uri::KeyValuePairList_t &keyValuePairs, const std::string &key) {
    for (const auto &elem : keyValuePairs)
        if (elem.first == key)
            return elem.second;

    return {};
}

/*static*/ std::string Uri::encode(const std::string &str) {
    static const std::string allowed = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.~";

    std::stringstream bruce;
    bruce << std::hex << std::setfill('0');

    for (const auto &c : str) {
        if (allowed.find(c) != std::string::npos)
            bruce << c;
        else
            bruce << '%' << std::setw(2) << std::uppercase << int(c) << std::nouppercase;
    }

    return bruce.str();
}

/*static*/ std::string Uri::decode(const std::string &str) {
    std::stringstream bruce;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '%' && i + 3 <= str.size()) {
            bruce << static_cast<char>(std::stoul(str.substr(i + 1, 2), nullptr, 16));
            i += 2;
        }
        else
            bruce << str[i];
    }

    return bruce.str();
}

/*static*/ std::string Uri::pathCombine(const std::string &a, const std::string &b) {
    // a needs no separator if it is empty or if last character is separator
    bool aNeedsSep = !a.empty() && a[a.size() - 1] != '/';
    // b has separator if it is not empty and the first char is separator
    bool bHasSep = !b.empty() && b[0] == '/';

    // add separator to a if it needs one, remove one from b if it has one
    return (aNeedsSep ? a + "/" : a) + (bHasSep ? b.substr(1) : b);

}

Uri::Uri(const std::string &uri) {
    auto [ schemaValue, schemaRemaining ] = splitTwo(uri, "://");
    schema(schemaValue);

    auto [ hostValue, hostRemaining ] = splitTwo(schemaRemaining, "/");
    host(hostValue);

    auto [ pathValue, pathRemaining ] = splitTwo(hostRemaining, "?");
    path(pathValue);

    auto [ queryValue, fragmentValue ] = splitTwo(pathRemaining, "#");
    query(queryValue);
    fragment(fragmentValue);
}

std::string Uri::path() const {
    return Str::joinAll(mPathParts, "/");
}

void Uri::path(const std::string &value) {
    mPathParts = Str::splitAll(value, "/", 0, false);
}

std::string Uri::query() const {
    return joinKeyValuePairs(mQueryParts);
}

void Uri::query(const std::string &value) {
    mQueryParts = splitKeyValuePairs(value);
}

std::optional<std::string> Uri::queryValue(const std::string &key) const {
    return findValueByKey(mQueryParts, key);
}

std::string Uri::fragment() const {
    return joinKeyValuePairs(mFragmentParts);
}

void Uri::fragment(const std::string &value) {
    mFragmentParts = splitKeyValuePairs(value);
}

std::optional<std::string> Uri::fragmentValue(const std::string &key) const {
    return findValueByKey(mFragmentParts, key);
}

std::string Uri::str() const {
    std::stringstream bruce;
    bruce << schema() << "://" << host() << "/";

    auto pathValue = path();
    if (!pathValue.empty())
        bruce << pathValue;

    auto queryValue = query();
    if (!queryValue.empty())
        bruce << "?" << queryValue;

    auto fragmentValue = fragment();
    if (!fragmentValue.empty())
        bruce << "#" << fragmentValue;

    return bruce.str();
}
