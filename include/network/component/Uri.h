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

#ifndef COMMONS_URI_H
#define COMMONS_URI_H

#include <optional>
#include <string>
#include <vector>

class Uri {
public:
    using KeyValuePair_t = std::pair<std::string, std::string>;
    using KeyValuePairList_t = std::vector<KeyValuePair_t>;

    static std::string encode(const std::string &str);
    static std::string decode(const std::string &str);
    static std::string pathCombine(const std::string &a, const std::string &b);

    explicit Uri(const std::string &uri);

    const std::string &schema() const {
        return mSchema;
    }
    void schema(const std::string &value) {
        mSchema = value;
    }

    const std::string &host() const {
        return mHost;
    }
    void host(const std::string &value) {
        mHost = value;
    }

    std::string path() const;
    void path(const std::string &value);

    const std::vector<std::string> &pathParts() const {
        return mPathParts;
    }
    std::vector<std::string> &pathParts() {
        return mPathParts;
    }

    std::string query() const;
    void query(const std::string &value);

    const KeyValuePairList_t &queryParts() const {
        return mQueryParts;
    }
    KeyValuePairList_t &queryParts() {
        return mQueryParts;
    }

    std::optional<std::string> queryValue(const std::string &key) const;
    std::string queryValue(const std::string &key, const std::string &fallback) {
        return queryValue(key).value_or(fallback);
    }

    std::string fragment() const;
    void fragment(const std::string &value);

    const KeyValuePairList_t &fragmentParts() const {
        return mFragmentParts;
    }
    KeyValuePairList_t &fragmentParts() {
        return mFragmentParts;
    }

    std::optional<std::string> fragmentValue(const std::string &key) const;
    std::string fragmentValue(const std::string &key, const std::string &fallback) {
        return fragmentValue(key).value_or(fallback);
    }

    std::string str() const;

protected:
    std::string mSchema;
    std::string mHost;

    std::vector<std::string> mPathParts;
    KeyValuePairList_t mQueryParts;
    KeyValuePairList_t mFragmentParts;
};

#endif //COMMONS_URI_H
