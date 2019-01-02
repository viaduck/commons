/*
 * Copyright (C) 2015-2019 The ViaDuck Project
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

#include "native/Native.h"
#include "socket/SSLSocket.h"
#include "Resolve.h"

#include <network/Connection.h>
#include <network/ConnectionInfo.h>


// global one-time network init on a per-platform basis
Native::Init gInit;
// thread specific SSL context
thread_local SSLContext SSLContext::mInstance;

void Connection::connect() {
    // resolve hostname
    Resolve resolve(mInfo.host(), mInfo.port());

    // try to connect to each
    for (addrinfo *it; resolve.next(it); ) {
        Socket_ref socket(mInfo.ssl() ? new SSLSocket(mInfo) : new TCPSocket(mInfo));

        if (socket->connect(it)) {
            mSocket = std::move(socket);
            return;
        }
    }

    throw connection_error("No connectable address could be resolved");
}

bool Connection::read(Buffer &buffer, uint32_t size) {
    if (!connected())
        return false;

    uint32_t total = 0;
    ssize_t read = 1;
    buffer.increase(size, true);

    while (total < size && read > 0) {
        if ((read = mSocket->read(buffer.data(buffer.size()), size - total)) > 0) {
            total += read;
            buffer.use(static_cast<uint32_t>(read));
        }
    }

    return total == size;
}

bool Connection::write(const Buffer &buffer) {
    if (!connected())
        return false;

    // try to write
    ssize_t res = mSocket->write(buffer.const_data(), buffer.size());
    if (res <= 0)
        return false;

    // succeed if all bytes were written
    return static_cast<uint32_t>(res) == buffer.size();
}

void Connection::disconnect() {
    mSocket.reset();
}