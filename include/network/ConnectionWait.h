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

#ifndef COMMONS_CONNECTIONWAIT_H
#define COMMONS_CONNECTIONWAIT_H

#include <network/Connection.h>

DEFINE_ERROR(connection_wait, connection_error);

/**
 * Platform independent TCP/SSL connection wait with notify support.
 * Enables indefinite waiting for a connection to become readable/connected or a notify to be sent
 */
class ConnectionWait {
    using ConnectionList_t = std::vector<std::weak_ptr<Connection>>;
public:
    enum class State {
        Readable,
        Connected,
        Exception,
    };

    using Ref = std::shared_ptr<ConnectionWait>;
    using ConnectionCallback_t = std::function<bool(const Connection::Ref &, State)>;
    using NotifyCallback = std::function<bool()>;

    explicit ConnectionWait() {
        // create notify socket
        connectNotify();
    }

    /**
     * Register a connection for being waited on in all future waits
     *
     * The connection will be saved internally as a weak_ptr. Destroyed connections will automatically be removed
     * from the list in the beginning of every wait.
     *
     * @param connection The connection to register for waiting
     */
    void registerConnection(const Connection::Ref &connection);

    /**
     * Sends a notify to a currently running/future wait.
     *
     * If a thread is currently waiting, it will wake up with a notify set, otherwise the next call to wait will return
     * immediately with a notify set.
     */
    void notify();

    /**
     * Wait indefinitely until a socket event is raised for one of the registered connections or a notify is set.
     * If a socket event or a notify already exists, wait will return immediately.
     *
     * Already connected connections will wait for the underlying socket to become readable, while disconnected
     * connections will wait for connected/exception states.
     *
     * Note: Since EOF and some other events are also reported as readable, "readable" does not actually guarantee that
     * any application data is available.
     *
     * @param notifyCallback On success, called (at most once) if a notify was set after waiting
     * @param connectionCallback On success, called (zero to N times) for each connection that received an event.
     * If multiple events were received simultaneously, the cb may be called multiple times for the same connection.
     * @return True on success, false otherwise
     */
    bool wait(const NotifyCallback &notifyCallback, const ConnectionCallback_t &connectionCallback);

protected:
    void connectNotify();
    void clearNotify();

    // special socket used for thread-safe wake up of wait
    std::unique_ptr<ISocket> mNotify;
    // registered connections
    ConnectionList_t mConnections;
};

#endif //COMMONS_CONNECTIONWAIT_H
