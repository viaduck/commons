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

#include <network/ConnectionWait.h>

#include "native/Native.h"
#include "socket/TCPSocket.h"
#include "socket/NotifySocket.h"
#include "socket/SocketWait.h"

using ForeachSocket_cb = bool(uint32_t index, const Connection::Ref &connection, const TCPSocket *socket);
/**
 * Iterate all Connection weak_ptr in specified list and either remove the pointer if it is expired,
 * or call cb otherwise.
 *
 * @param connections List of connection weak_ptr to iterate over. Expired weak_ptr will be removed on success
 * @param cb Optional callback used for every unexpired Connection.
 * The index specifies the index of the connection in the initial list of weak_ptr (before removals).
 * The socket is set if the connection has a tcp+ socket, nullptr otherwise.
 * If any callback returns false, foreachSocket returns an error.
 * @return True on success, false otherwise.
 */
static bool foreachSocket(std::vector<std::weak_ptr<Connection>> &connections,
                          const std::function<ForeachSocket_cb> &cb = {}) {
    uint32_t index = 0;
    for (auto it = connections.begin(); it != connections.end(); index++) {
        if (auto connection = it->lock()) {
            auto tcp_sock = dynamic_cast<TCPSocket *>(connection->socket());
            if (cb && !cb(index, connection, tcp_sock))
                return false;

            it++;
        }
        else
            it = connections.erase(it);
    }

    return true;
}

bool ConnectionWait::wait(const NotifyCallback &notifyCallback, const ConnectionCallback_t &connectionCallback) {
    // filter expired weak pointers out of connections
    foreachSocket(mConnections);
    // deep copy to avoid modifications of mConnections breaking the synchronization of entries and connections
    ConnectionList_t connections = mConnections;

    // get notify socket pointer
    auto notify_sock = dynamic_cast<NotifySocket*>(mNotify.get());
    L_assert(notify_sock, connection_wait_error);

    // add notify socket as entry 0 -> wait for readable
    std::vector<SocketWait::Entry> entries = { SocketWait::Entry(notify_sock->fd(), SocketWait::Events::READABLE) };
    // add one wait entry per connection
    foreachSocket(connections, [&] (uint32_t, const Connection::Ref &connection, const TCPSocket *tcpSocket) {
        if (!tcpSocket)
            // disconnected -> just add placeholder entry
            entries.emplace_back();
        else if (connection->connected())
            // connected -> wait for readable
            entries.emplace_back(tcpSocket->fd(), SocketWait::Events::READABLE);
        else
            // not connected yet -> wait for writeable/exception
            entries.emplace_back(tcpSocket->fd(), SocketWait::Events::WRITEABLE | SocketWait::Events::EXCEPT);

        return true;
    });

    // indefinite wait for one of sockets to become readable or a connection to succeed/fail
    if (SocketWait::wait(entries) == NetworkResultType::SUCCESS) {
        // clear and call notify cb if notify was set
        if (entries.at(0).readable()) {
            clearNotify();

            if (!notifyCallback())
                return false;
        }

        // call connection cb for each connection that has some response set
        return foreachSocket(connections, [&] (uint32_t index, const Connection::Ref &connection, const TCPSocket *) {
            const auto &entry = entries.at(index + 1);

            if ((entry.except() && !connectionCallback(connection, State::Exception)) ||
                (entry.writeable() && !connectionCallback(connection, State::Connected)) ||
                (entry.readable() && !connectionCallback(connection, State::Readable)))
                return false;

            return true;
        });
    }

    return false;
}

void ConnectionWait::notify() {
    auto notify_sock = dynamic_cast<NotifySocket*>(mNotify.get());
    if (notify_sock)
        notify_sock->notify();
}

void ConnectionWait::clearNotify() {
    auto notify_sock = dynamic_cast<NotifySocket*>(mNotify.get());
    if (notify_sock)
        notify_sock->clear();
}

void ConnectionWait::registerConnection(const Connection::Ref &connection) {
    mConnections.emplace_back(connection);
}

void ConnectionWait::connectNotify() {
    // create platform specific notify socket
    mNotify = std::make_unique<NotifySocket>();
    mNotify->connect(nullptr);
}
