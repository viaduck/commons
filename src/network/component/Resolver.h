/*
 * Copyright (C) 2019-2025 The ViaDuck Project
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

#include <network/component/NetworkResult.h>

#include "../native/Native.h"

class Resolver {
    using addrinfo_ref = std::unique_ptr<addrinfo, decltype(&Native::freeaddrinfo)>;
public:
    Resolver() : mResult(nullptr, &Native::freeaddrinfo) { }

    /// reset resolver for new resolution
    void reset() {
        mResult.reset();
        mIterator = nullptr;
    }

    /// resolve hostname/port to ip addresses, if no result is currently loaded
    NetworkResult resolve(const std::string &host, uint16_t port) {
        if (mResult)
            return NetworkResultType::SUCCESS;

        return resolveInternal(host, port);
    }

    /// get current address if exists
    bool current(addrinfo *&out) {
        if (!mResult || !mIterator)
            return false;

        out = mIterator;
        return true;
    }

    /// advance iterator
    void advance() {
        if (mResult && mIterator && mIterator->ai_next)
            mIterator = mIterator->ai_next;
        else
            mIterator = nullptr;
    }

protected:
    NetworkResult resolveInternal(const std::string &host, uint16_t port) {
        addrinfo query{}, *result;
        // query all families (v4 and v6)
        query.ai_family = PF_UNSPEC;
        // query only TCP
        query.ai_socktype = SOCK_STREAM;
        query.ai_protocol = IPPROTO_TCP;

        // resolve hostname or fail
        if (int res = Native::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &query, &result); res != 0)
            return NetworkResolveError(res);

        mResult.reset(result);
        mIterator = result;
        return NetworkResultType::SUCCESS;
    }

    addrinfo_ref mResult;
    addrinfo *mIterator = nullptr;
};

#endif //COMMONS_RESOLVE_H
