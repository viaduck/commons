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

#ifndef COMMONS_NOTIFYSOCKET_H
#define COMMONS_NOTIFYSOCKET_H

#include <network/socket/ISocket.h>

DEFINE_ERROR(notify_socket, base_error);

class NotifySocket : public ISocket {
public:
    explicit NotifySocket() = default;

    ~NotifySocket() override {
        Native::close(mRxSocket);
        Native::close(mTxSocket);
    }

    /// @return Underlying rx socket
    SOCKET fd() const {
        return mRxSocket;
    }

#ifdef WIN32
    bool connect(addrinfo *) override {
        // temporary listen socket
        SOCKET sock = Native::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        L_assert(sock != INVALID_SOCKET, notify_socket_error);

        // enable reusing old sockaddr
        int value = 1;
        L_assert(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(value)) == 0, notify_socket_error);

        // listen for a single connection on IPv4 loopback, any port
        sockaddr_in inaddr {};
        memset(&inaddr, 0, sizeof(inaddr));
        inaddr.sin_family = AF_INET;
        inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        inaddr.sin_port = 0;
        L_assert(bind(sock, (sockaddr*)&inaddr, sizeof(inaddr)) == 0, notify_socket_error);
        L_assert(listen(sock, 1) == 0, notify_socket_error);

        // get listen socket address for connect
        sockaddr addr {};
        memset(&addr, 0, sizeof(addr));
        int len = sizeof(addr);
        L_assert(getsockname(sock, &addr, &len) == 0, notify_socket_error);

        // connect RX socket
        mRxSocket = Native::socket(AF_INET, SOCK_STREAM, 0);
        L_assert(mRxSocket != INVALID_SOCKET, notify_socket_error);
        L_assert(Native::connect(mRxSocket, &addr, len) == 0, notify_socket_error);

        // accept TX socket
        mTxSocket = accept(sock, nullptr, nullptr);
        L_assert(mTxSocket != INVALID_SOCKET, notify_socket_error);

        // clean up temp socket
        Native::close(sock);
        return true;
    }
#else
    bool connect(addrinfo *) override {
        SOCKET fds[2];
        // create the two connected sockets
        L_assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, notify_socket_error);

        // save descriptors
        mRxSocket = fds[0];
        mTxSocket = fds[1];
        return true;
    }
#endif

    /// Sends one byte to notify the receiving end
    void notify() {
        std::unique_lock<std::mutex> lock(mMutex);
        mNotifyCount++;

        uint8_t message = 1;
        L_assert(write(&message, sizeof(message)) == sizeof(message), notify_socket_error);
    }

    /// Clears a notification
    void clear() {
        std::unique_lock<std::mutex> lock(mMutex);

        for (uint32_t i = 0; i < mNotifyCount; i++) {
            uint8_t message;
            L_assert(read(&message, sizeof(message)) == sizeof(message), notify_socket_error);
        }

        mNotifyCount = 0;
    }

    int64_t read(void *data, uint32_t size) override {
        return Native::recv(mRxSocket, data, size);
    }

    int64_t write(const void *data, uint32_t size) override {
        return Native::send(mTxSocket, data, size);
    }

protected:
    SOCKET mRxSocket = INVALID_SOCKET, mTxSocket = INVALID_SOCKET;

    std::mutex mMutex;
    uint32_t mNotifyCount = 0;
};

#endif //COMMONS_NOTIFYSOCKET_H
