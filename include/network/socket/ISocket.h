/*
 * Copyright (C) 2019 The ViaDuck Project
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

#ifndef COMMONS_ISOCKET_H
#define COMMONS_ISOCKET_H

#include <enum/network/IPProtocol.h>
#include <network/component/ConnectionInfo.h>

struct addrinfo;

class ISocket {
public:
    explicit ISocket() = default;
    explicit ISocket(ConnectionInfo info) : mInfo(std::move(info)) { };
    virtual ~ISocket() = default;

    virtual bool connect(addrinfo *addr) = 0;
    virtual int64_t read(void *data, uint32_t size) = 0;
    virtual int64_t write(const void *data, uint32_t size) = 0;

    IPProtocol protocol() const {
        return mProtocol;
    }

protected:
    // constant connection information
    ConnectionInfo mInfo;
    // current IP protocol used
    IPProtocol mProtocol = IPProtocol::VALUE_INVALID;
};

#endif //COMMONS_ISOCKET_H
