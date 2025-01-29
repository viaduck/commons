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

#include "component/Resolver.h"
#include "native/Native.h"
#include "socket/DefaultSocketFactory.h"
#include "secure_memory/String.h"

#include <network/Connection.h>

// global one-time network init on a per-platform basis
Native::Init gInit;
// thread specific SSL context
thread_local SSLContext SSLContext::mInstance;

Connection::Connection(ConnectionInfo connectionInfo) : mInfo(std::move(connectionInfo)),
        mSocketFactory(std::make_unique<DefaultSocketFactory>()), mResolver(std::make_unique<Resolver>()) {

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

NetworkResult Connection::connectNonBlocking() {
    if (connected())
        return NetworkResultType::SUCCESS;

    // resolve and try to get current address
    addrinfo *addr;
    if (auto rv = mResolver->resolve(mInfo.host(), mInfo.port()); !rv)
        return rv;
    if (!mResolver->current(addr)) {
        mResolver->reset();
        return NetworkNotConnectableError();
    }

    // create socket if needed
    if (!mSocket)
        mSocket.reset(mSocketFactory->create(mInfo));

    // non-blocking connect only supported by TCPSocket and subclasses
    if (auto *tcpSocket = dynamic_cast<TCPSocket*>(mSocket.get())) {
        // try to connect
        NetworkResult rv = tcpSocket->connectNonBlocking(addr);
        if (rv == NetworkResultType::SUCCESS) {
            // connect success
            mResolver->reset();
            mConnected = true;
        }
        else if (!rv) {
            // connect failure, advance resolve address
            mResolver->advance();

            // retry to connect to the next address
            return NetworkResultType::RETRY;
        }

        // success or deferred
        return rv;
    }

    // unrecoverable error: socket unsupported
    return NetworkUnsupportedSocketError();
}

void Connection::connect() {
    if (connected())
        return;

    // resolve hostname to ip addresses
    if (Resolver resolver; resolver.resolve(mInfo.host(), mInfo.port())) {
        // try to connect to each
        for (addrinfo *it; resolver.current(it); resolver.advance()) {
            // try to connect to socket
            if (std::unique_ptr<ISocket> socket(mSocketFactory->create(mInfo)); socket->connect(it)) {
                mSocket = std::move(socket);
                mConnected = true;

                return;
            }
        }

        throw connection_error("No resolved address was connectable for " +
                mInfo.host() + ":" + std::to_string(mInfo.port()));
    }

    throw resolve_error("Hostname resolution failed for " + mInfo.host() + ":" + std::to_string(mInfo.port()));
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

NetworkResult Connection::readNonBlocking(Buffer &buffer, uint32_t size) {
    if (!connected())
        return NetworkInternalError();

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

    // no data available -> wait, we read some data -> success, error/disconnect -> error
#ifdef _WIN32
    if (read == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
#else
    if (read == SOCKET_ERROR && (errno == EINPROGRESS || errno == EAGAIN))
#endif
        return NetworkResultType::WAIT_EVENT;
    else if (read > 0)
        return NetworkResultType::SUCCESS;

    return NetworkOSError();
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
