/*
 * Copyright (C) 2015-2023 The ViaDuck Project
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
#include "socket/DefaultSocketFactory.h"
#include "Resolve.h"
#include "secure_memory/String.h"

#include <network/Connection.h>

// global one-time network init on a per-platform basis
Native::Init gInit;
// thread specific SSL context
thread_local SSLContext SSLContext::mInstance;

class Connection::State {
public:
    int getCurrentAddr(const ConnectionInfo &info, addrinfo *&out) {
        if (!resolve) {
            // create resolve and advance to first addr if not exists
            try {
                resolve = std::make_unique<Resolve>(info.host(), info.port());
                resolve->advance();
            }
            catch (const resolve_error &) {
                // unrecoverable error: resolving failed
                resolve.reset();
                return -2;
            }
        }
        // try to get the current addr from resolve
        if (!resolve->current(out)) {
            // error if current is null
            resolve.reset();
            return -1;
        }

        return 1;
    }

    int advanceNextAddr() {
        // try to advance to the next resolved addr
        if (!resolve->advance()) {
            // error if no more addr in list
            resolve.reset();
            return -1;
        }

        return 1;
    }

    // state of resolve
    std::unique_ptr<Resolve> resolve;
};

Connection::Connection(ConnectionInfo connectionInfo)
        : mInfo(std::move(connectionInfo)), mSocketFactory(new DefaultSocketFactory), mState(new State()) {

}
Connection::Connection(const std::string &host, uint16_t port, bool ssl)
        : Connection(ConnectionInfo(host, port, ssl)) {

}
Connection::~Connection() = default;

bool Connection::connected() const {
    return static_cast<bool>(mSocket) && mConnected;
}

IPProtocol Connection::protocol() const {
    return connected() ? mSocket->protocol() : IPProtocol::VALUE_INVALID;
}

ISocket *Connection::socket() {
    return mSocket.get();
}

int Connection::connectNonBlocking() {
    if (connected())
        return 1;

    // create resolve if needed, get current addr
    addrinfo *addr;
    int rv = mState->getCurrentAddr(mInfo, addr);
    if (rv < 0)
        return rv;

    // create socket if needed
    if (!mSocket)
        mSocket.reset(mSocketFactory->create(mInfo));

    // non-blocking connect only supported by TCPSocket and subclasses
    if (auto *tcpSocket = dynamic_cast<TCPSocket*>(mSocket.get())) {
        // try to connect
        rv = tcpSocket->connectNonBlocking(addr);
        if (rv > 0) {
            // connect success
            mConnected = true;
            mState->resolve.reset();
        }
        else if (rv < 0) {
            // connect failure, try to advance resolve address
            rv = mState->advanceNextAddr();
            if (rv < 0)
                // no more resolvable addresses -> error
                return -1;

            // return 0 leads to trying the next addr
            rv = 0;
        }

        // 1 or 0
        return rv;
    }

    // unrecoverable error: socket unsupported
    return -2;
}

void Connection::connect() {
    if (connected())
        return;

    // resolve hostname
    Resolve resolve(mInfo.host(), mInfo.port());

    // try to connect to each
    for (addrinfo *it; resolve.advance() && resolve.current(it); ) {
        std::unique_ptr<ISocket> socket(mSocketFactory->create(mInfo));

        if (socket->connect(it)) {
            mSocket = std::move(socket);
            mConnected = true;
            return;
        }
    }

    throw connection_error("No connectable address could be resolved");
}

bool Connection::tryConnect() {
    try {
        connect();
        return true;
    }
    catch (const resolve_error &e) {
        Log::dbg << "Resolve error occurred: " << e.what();
    }
    catch (const socket_error &e) {
        Log::dbg << "Socket error occurred: " << e.what();
    }
    catch (const connection_error &e) {
        Log::dbg << "Connection error occurred: " << e.what();
    }
    return false;
}

void Connection::disconnect() {
    mConnected = false;
    mSocket.reset();
}

bool Connection::read(Buffer &buffer, uint32_t size) {
    if (!connected())
        return false;

    uint32_t total = 0;
    int64_t read = 1;
    buffer.increase(size, true);

    while (total < size && read > 0) {
        if ((read = mSocket->read(buffer.data(buffer.size()), size - total)) > 0) {
            total += read;
            buffer.use(static_cast<uint32_t>(read));
        }
    }

    return total == size;
}

int Connection::readNonBlocking(Buffer &buffer, uint32_t size) {
    if (!connected())
        return -1;

    // only tcp+ is supported
    auto tcp_sock = dynamic_cast<TCPSocket*>(socket());
    L_assert(tcp_sock, connection_error);

    uint32_t total = 0;
    int64_t read = 1;
    buffer.increase(size, true);

    tcp_sock->setNonBlocking(true);
    while (total < size && read > 0) {
        if ((read = mSocket->read(buffer.data(buffer.size()), size - total)) > 0) {
            total += read;
            buffer.use(static_cast<uint32_t>(read));
        }
    }
    tcp_sock->setNonBlocking(false);

    // no data available -> retry, we read some data -> success, error/disconnect -> error
#ifdef _WIN32
    if (read == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
#else
    if (read == SOCKET_ERROR && (errno == EINPROGRESS || errno == EAGAIN))
#endif
        return 0;
    else if (read > 0)
        return 1;
    else
        return -1;
}

bool Connection::write(const Buffer &buffer) {
    if (!connected())
        return false;

    // try to write
    int64_t res = mSocket->write(buffer.const_data(), buffer.size());
    if (res <= 0)
        return false;

    // succeed if all bytes were written
    return static_cast<uint32_t>(res) == buffer.size();
}
