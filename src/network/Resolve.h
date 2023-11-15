/*
 * Copyright (C) 2019-2023 The ViaDuck Project
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

#ifndef COMMONS_RESOLVE_H
#define COMMONS_RESOLVE_H

DEFINE_ERROR(resolve, base_error);

class Resolve {
    using addrinfo_ref = std::unique_ptr<addrinfo, decltype(&Native::freeaddrinfo)>;
public:
    Resolve(const std::string &host, uint16_t port) : mResult(nullptr, &Native::freeaddrinfo) {
        addrinfo query{}, *result;

        // query all families (v4 and v6)
        query.ai_family = PF_UNSPEC;
        // query only TCP
        query.ai_socktype = SOCK_STREAM;
        query.ai_protocol = IPPROTO_TCP;

        // resolve hostname
        int res = Native::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &query, &result);
        if (res == 0 && result)
            mResult.reset(result);
        else
            throw resolve_error("Resolve error: " + std::to_string(res));
    }

    bool current(addrinfo *&out) {
        if (!mResult || !mIterator)
            return false;

        out = mIterator;
        return true;
    }

    bool advance() {
        // advance iterator
        if (mResult && !mIterator)
            mIterator = mResult.get();
        else if (mResult && mIterator->ai_next)
            mIterator = mIterator->ai_next;
        else {
            mIterator = nullptr;
            return false;
        }

        return true;
    }

protected:
    addrinfo_ref mResult;
    addrinfo *mIterator = nullptr;
};

#endif //COMMONS_RESOLVE_H
