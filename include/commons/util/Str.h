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
#ifndef COMMONS_STR_H
#define COMMONS_STR_H

#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

/**
 * Utility for common string operations
 */
class Str {
public:
    /**
     * Split string at delimiter into parts
     *
     * @param str String to split into parts
     * @param delim Delimiter to split at, must not be empty
     * @param maxParts Maximum number of parts to return. The last part may contain delim if maxParts has been reached
     * @param allowEmpty If false, empty parts will not be returned and will not be counted towards maxParts
     * @return Vector of split parts without delim
     */
    static inline std::vector<std::string> splitAll(const std::string &str, const std::string &delim,
                                                    size_t maxParts = 0, bool allowEmpty = true) {
        std::vector<std::string> result;
        size_t next, last = 0, count = 0;

        // protect against empty delimiter
        if (delim.empty())
            return {};

        // only enter loop if a next delimiter was found, and adding it + left-overs would not exceed max parts
        while ((next = str.find(delim, last)) != std::string::npos && (maxParts == 0 || count + 1 < maxParts)) {
            if (allowEmpty || last < next)
                result.emplace_back(str.substr(last, next - last));

            last = next + delim.size();
            count++;
        }

        // add left-over portion
        if (allowEmpty || last < str.size())
            result.emplace_back(str.substr(last));

        return result;
    }

    /**
     * Join all parts into one string with delimiter
     *
     * @param parts Vector of parts to be joined
     * @param delim Delimiter to insert in between parts when joining
     * @return Joined string
     */
    static inline std::string joinAll(const std::vector<std::string> &parts, const std::string &delim) {
        std::stringstream bruce;

        for (size_t i = 0; i < parts.size(); i++)
            bruce << (i ? delim : "") << parts[i];

        return bruce.str();
    }
};

#endif //COMMONS_STR_H
