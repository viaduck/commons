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

#include "native/Native.h"
#include "socket/SSLSocket.h"
#include "socket/NotifySocket.h"

#include <network/PushConnection.h>

bool PushConnection::wait(bool &readable, bool &notify) {
    // create a set of sockets for select, add our socket and the notify
    fd_set set;
    FD_ZERO(&set);

    // connection socket, notify socket
    SOCKET sfd = INVALID_SOCKET, nfd;

    // obtain connection socket, only tcp+ waiting supported
    if (connected()) {
        auto tcp_sock = dynamic_cast<TCPSocket *>(socket());
        L_assert(tcp_sock, async_connection_error);
        sfd = tcp_sock->fd();
        FD_SET(sfd, &set);
    }

    // obtain notify socket
    auto notify_sock = dynamic_cast<NotifySocket*>(mNotify.get());
    L_assert(notify_sock, async_connection_error);
    nfd = notify_sock->fd();
    FD_SET(nfd, &set);

#ifdef WIN32
    // ignored on windows
    int maxfd = 0;
#else
    // max socket fd
    int maxfd = std::max(sfd, nfd);
#endif

    // indefinite wait for one of sockets to become readable
    if (Native::select(maxfd + 1, &set, nullptr, nullptr, nullptr) >= 0) {
        // socket is readable
        readable = FD_ISSET(sfd, &set);
        // notify was sent
        notify = FD_ISSET(nfd, &set);

        return true;
    }

    // error
    return false;
}

int PushConnection::readNonBlocking(Buffer &buffer, uint32_t size) {
    if (!connected())
        return -1;

    // only tcp+ is supported
    auto tcp_sock = dynamic_cast<TCPSocket*>(socket());
    L_assert(tcp_sock, async_connection_error);

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
#ifdef WIN32
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

void PushConnection::notify() {
    auto notify_sock = dynamic_cast<NotifySocket*>(mNotify.get());
    if (notify_sock)
        notify_sock->notify();
}

void PushConnection::clearNotify() {
    auto notify_sock = dynamic_cast<NotifySocket*>(mNotify.get());
    if (notify_sock)
        notify_sock->clear();
}

void PushConnection::connectNotify() {
    // create platform specific notify socket
    mNotify = std::make_unique<NotifySocket>(mInfo);
    mNotify->connect(nullptr);
}
