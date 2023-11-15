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

#ifndef COMMONS_TCPSOCKET_H
#define COMMONS_TCPSOCKET_H

#include <commons/util/Timer.h>
#include <network/socket/ISocket.h>
#include "SocketWait.h"

DEFINE_ERROR(socket, base_error);

class TCPSocket : public ISocket {
public:
    class NonBlockingScope {
    public:
        explicit NonBlockingScope(TCPSocket *socket) : mSocket(socket) {
            socket->setNonBlocking(true);
        }
        ~NonBlockingScope() {
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

    int initConnect(addrinfo *addr) {
        // try to create socket
        mSocket = Native::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        L_assert(mSocket != INVALID_SOCKET, socket_error);

        // set r/w timeout
        setTimeoutIO(mInfo.timeoutIO());
        // set IP protocol info
        setProtocol(addr->ai_family);
        // set non-blocking now, set blocking when leaving scope
        NonBlockingScope nonBlockingScope(this);

        // connect and return immediately
        int res = Native::connect(mSocket, addr->ai_addr, addr->ai_addrlen);
        if (res == 0) {
            // connect success
            return 1;
        }
#ifdef WIN32
        else if (res == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        else if (res == SOCKET_ERROR && errno == EINPROGRESS) {
#endif
            // retry later to complete connect
            return 0;
        }

        // error
        return -1;
    }
    int finishConnect(const std::optional<int32_t> &timeoutMs) {
        NonBlockingScope nonBlockingScope(this);

        SocketWait::Entry waitEntry(mSocket, SocketWait::Events::WRITEABLE | SocketWait::Events::EXCEPT);
        auto rv = SocketWait::waitOne(waitEntry, timeoutMs);
        if (rv > 0) {
            // connect completed - successfully or not

            // exception in socket
            if (waitEntry.except())
                return -1;

            // writeable - check socket error
            if (extractError() != 0)
                return -1;

            // success
            return 1;
        }

        // timeout or error
        return rv;
    }

    int startConnectNonBlocking(addrinfo *addr) {
        // init connection attempt
        auto rv = initConnect(addr);
        if (rv < 0) {
            // disconnect on error
            disconnect();
        }
        else if (rv == 0) {
            // now the connection process is active
            mConnectActive = true;
            // start timer on "wait" with timeout
            if (mInfo.timeoutConnect() > 0)
                mTimeout.start(mInfo.timeoutConnect());
        }

        return rv;
    }
    int continueConnectNonBlocking() {
        // continue connection attempt
        auto rv = finishConnect(0);
        if (rv < 0) {
            // disconnect on error
            disconnect();
        }
        if (rv != 0) {
            // success or failure, reset state
            mTimeout.reset();
            mConnectActive = false;
        }

        return rv;
    }
    int timeoutConnectNonBlocking() {
        // timeout in connection attempt, last try to complete connection
        auto rv = finishConnect(0);
        if (rv <= 0) {
            // timeout is an error now
            disconnect();
            rv = -1;
        }

        // reset state
        mTimeout.reset();
        mConnectActive = false;
        return rv;
    }

    virtual int connectNonBlocking(addrinfo *addr) {
        if (!mConnectActive)
            return startConnectNonBlocking(addr);
        else if (!mTimeout.active() || mTimeout.running())
            return continueConnectNonBlocking();
        else
            return timeoutConnectNonBlocking();
    }

    bool connect(addrinfo *addr) override {
        auto rv = initConnect(addr);
        if (rv == 1) {
            // immediately connected
            return true;
        }
        else if (rv == 0) {
            // awaiting result, wait until timeout
            auto timeoutMs = static_cast<int32_t>(mInfo.timeoutConnect());
            return finishConnect(timeoutMs == 0 ? std::optional<int32_t>{} : timeoutMs) > 0;
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
    void setNonBlocking(bool value) {
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
    SOCKET fd() const {
        return mSocket;
    }

protected:
    /**
     * Sets the send and receive timeouts.
     *
     * @param t Timeout in milliseconds
     */
    void setTimeoutIO(uint32_t t) {
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

    int extractError() const {
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
