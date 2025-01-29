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

#ifndef COMMONS_TCPSOCKET_H
#define COMMONS_TCPSOCKET_H

#include <commons/util/Timer.h>
#include <network/socket/ISocket.h>
#include "SocketWait.h"

DEFINE_ERROR(socket, base_error);

class TCPSocket : public ISocket {
public:
    class NonBlockingGuard {
    public:
        [[maybe_unused]] explicit NonBlockingGuard(TCPSocket *socket) : mSocket(socket) {
            socket->setNonBlocking(true);
        }
        ~NonBlockingGuard() {
            mSocket->setNonBlocking(false);
        }

    protected:
        TCPSocket *mSocket;
    };

    explicit TCPSocket(const ConnectionInfo &info) : ISocket(info) { }

    ~TCPSocket() override {
        disconnect();
    }

    void disconnect() {
        if (mSocket != INVALID_SOCKET) {
            // this will gracefully shut down the connection
            Native::shutdown(mSocket, NW__SHUT_RDWR);
            Native::close(mSocket);
        }

        mSocket = INVALID_SOCKET;
    }

    NetworkResult initConnect(addrinfo *addr) {
        // try to create socket
        mSocket = Native::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        L_assert_ne(mSocket, INVALID_SOCKET, socket_error);

        // set r/w timeout
        setTimeoutIO(mInfo.timeoutIO());
        // set IP protocol info
        setProtocol(addr->ai_family);
        // set non-blocking now, set blocking when leaving scope
        [[maybe_unused]] NonBlockingGuard nonBlocking(this);

        // connect and return immediately
        int res = Native::connect(mSocket, addr->ai_addr, addr->ai_addrlen);
        if (res == 0) {
            // connect success
            return NetworkResultType::SUCCESS;
        }
#ifdef WIN32
        else if (res == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        else if (res == SOCKET_ERROR && errno == EINPROGRESS) {
#endif
            // wait for writeable event
            return NetworkResultType::WAIT_EVENT;
        }

        // error
        return NetworkOSError();
    }
    NetworkResult finishConnect(const std::optional<int32_t> &timeoutMs) {
        [[maybe_unused]] NonBlockingGuard nonBlocking(this);

        SocketWait::Entry waitEntry(mSocket, SocketWait::Events::WRITEABLE | SocketWait::Events::EXCEPT);
        auto rv = SocketWait::waitOne(waitEntry, timeoutMs);
        if (rv == NetworkResultType::SUCCESS) {
            // connect completed: successfully or not

            // exception in socket: should never happen
            if (waitEntry.except())
                return NetworkInternalError();

            // writeable: check socket error
            if (int errorCode = extractError(); errorCode != 0)
                return NetworkOSError(errorCode);

            // success
            return NetworkResultType::SUCCESS;
        }

        // timeout error or underlying error
        return rv;
    }

    NetworkResult startConnectNonBlocking(addrinfo *addr) {
        // init connection attempt
        auto rv = initConnect(addr)
                // map timeout error to "wait" result
                .timeoutToWait();

        if (!rv) {
            // error: disconnect
            disconnect();
        }
        else if (rv.isDeferred()) {
            // wait/retry: now the connection process is active
            mConnectActive = true;
            // start timer for timeout if specified
            if (mInfo.timeoutConnect() > 0)
                mTimeout.start(mInfo.timeoutConnect());
        }

        // success or error
        return rv;
    }
    NetworkResult continueConnectNonBlocking() {
        // continue connection attempt
        auto rv = finishConnect(0)
                // map timeout error to "wait" result
                .timeoutToWait();

        if (!rv) {
            // error: disconnect
            disconnect();
        }
        if (!rv || rv == NetworkResultType::SUCCESS) {
            // error, success: reset state
            mTimeout.reset();
            mConnectActive = false;
        }

        // success, timeout or error
        return rv;
    }
    NetworkResult timeoutConnectNonBlocking() {
        // timeout in connection attempt, last try to complete connection
        auto rv = finishConnect(0);

        // rv timeout is considered an error now
        if (!rv) {
            // error: disconnect
            disconnect();
        }

        // in any case: reset state
        mTimeout.reset();
        mConnectActive = false;
        return rv;
    }

    virtual NetworkResult connectNonBlocking(addrinfo *addr) {
        if (!mConnectActive)
            return startConnectNonBlocking(addr);
        else if (!mTimeout.active() || mTimeout.running())
            return continueConnectNonBlocking();
        else
            return timeoutConnectNonBlocking();
    }

    bool connect(addrinfo *addr) override {
        auto rv = initConnect(addr);
        if (rv == NetworkResultType::SUCCESS) {
            // immediately connected
            return true;
        }
        else if (rv.isDeferred()) {
            // awaiting result, wait until timeout
            auto timeoutMs = static_cast<int32_t>(mInfo.timeoutConnect());
            return finishConnect(timeoutMs == 0 ? std::optional<int32_t>{} : timeoutMs) == NetworkResultType::SUCCESS;
        }

        // error
        return false;
    }

    int64_t read(void *data, uint32_t size) override {
        return Native::recv(mSocket, data, size);
    }

    int64_t write(const void *data, uint32_t size) override {
        return Native::send(mSocket, data, size);
    }

    /**
     * Sets the non-blocking state.
     *
     * @param value True for non-blocking, false for blocking
     */
    void setNonBlocking(bool value) const {
#ifdef WIN32
        u_long mode = value ? 1 : 0;
        ioctlsocket(mSocket, FIONBIO, &mode);
#else
        int flags = fcntl(mSocket, F_GETFL, NULL);
        if (value)
            flags |= O_NONBLOCK;
        else
            flags &= ~O_NONBLOCK;
        fcntl(mSocket, F_SETFL, flags);
#endif
    }

    /**
     * @return Underlying socket
     */
    [[nodiscard]] SOCKET fd() const {
        return mSocket;
    }

protected:
    /**
     * Sets the send and receive timeouts.
     *
     * @param t Timeout in milliseconds
     */
    void setTimeoutIO(uint32_t t) const {
        if (t == 0)
            return;

#ifdef WIN32
        DWORD tv = t;
#else
        auto toc = static_cast<int32_t>(t);
        timeval tv = { .tv_sec = toc / 1000, .tv_usec = 1000 * (toc % 1000) };
#endif

        setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
        setsockopt(mSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
    }

    [[nodiscard]] int extractError() const {
        int error;
        socklen_t len = sizeof(error);

        if (Native::getsockopt(mSocket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len) != 0)
            return -1;

        return error;
    }

    void setProtocol(int ai_family) {
        if (ai_family == AF_INET)
            mProtocol = IPProtocol::IPv4;
        else if (ai_family == AF_INET6)
            mProtocol = IPProtocol::IPv6;
    }

    // internal socket
    SOCKET mSocket = INVALID_SOCKET;

    // internal non-blocking connect state
    Timer mTimeout;
    bool mConnectActive = false;
};

#endif //COMMONS_TCPSOCKET_H
