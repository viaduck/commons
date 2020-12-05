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

#ifndef COMMONS_TCPSOCKET_H
#define COMMONS_TCPSOCKET_H

#include <network/socket/ISocket.h>

DEFINE_ERROR(socket, base_error);

class TCPSocket : public ISocket {
public:
    explicit TCPSocket(const ConnectionInfo &info) : ISocket(info) { }

    ~TCPSocket() override {
        if (mSocket != INVALID_SOCKET) {
            // this will gracefully shut down the connection
            Native::shutdown(mSocket, NW__SHUT_RDWR);
            Native::close(mSocket);
        }
    }

    bool connect(addrinfo *addr) override {
        // try to create socket
        mSocket = Native::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        L_assert(mSocket != INVALID_SOCKET, socket_error);

        // set r/w timeout
        setTimeoutIO(mInfo.timeoutIO());
        // make socket non-blocking
        setNonBlocking(true);
        // set IP protocol info
        setProtocol(addr->ai_family);

        // connect and return immediately
        int res = Native::connect(mSocket, addr->ai_addr, addr->ai_addrlen);

        if (res == 0) {
            // connect success, make socket blocking again
            setNonBlocking(false);
            return true;
        }
#ifdef WIN32
        else if (res == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        else if (res == SOCKET_ERROR && errno == EINPROGRESS) {
#endif
            // create a set of sockets for select, add only our socket
            fd_set set;
            FD_ZERO(&set);
            FD_SET(mSocket, &set);

            // timeout in microseconds
            auto toc = static_cast<int32_t>(mInfo.timeoutConnect());
            timeval tv = {.tv_sec = toc / 1000, .tv_usec = 1000 * (toc % 1000)};
            // timeout -> pass NULL to block
            timeval *ptv = toc > 0 ? &tv : nullptr;

            // waits for socket to complete connect (become writeable)
            if (Native::select(mSocket + 1, nullptr, &set, nullptr, ptv) > 0) {
                // connect completed - successfully or not

                int error;
                socklen_t len = sizeof(error);
                if (Native::getsockopt(mSocket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len) == 0 && error == 0) {
                    // connect success, make socket blocking again
                    setNonBlocking(false);
                    return true;
                }
            }
        }

        // timeout or error
        return false;
    }

    ssize_t read(void *data, uint32_t size) override {
        return Native::recv(mSocket, data, size);
    }

    ssize_t write(const void *data, uint32_t size) override {
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

    void setProtocol(int ai_family) {
        if (ai_family == AF_INET)
            mProtocol = IPProtocol::IPv4;
        else if (ai_family == AF_INET6)
            mProtocol = IPProtocol::IPv6;
    }

    SOCKET mSocket = INVALID_SOCKET;
};

#endif //COMMONS_TCPSOCKET_H
