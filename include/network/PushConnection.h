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

#ifndef COMMONS_PUSHCONNECTION_H
#define COMMONS_PUSHCONNECTION_H

#include <network/Connection.h>

DEFINE_ERROR(async_connection, connection_error);

/**
 * Platform independent TCP/SSL client with non-blocking read and wait support
 */
class PushConnection : Connection {
public:
    explicit PushConnection(const ConnectionInfo &info) : Connection(info) {
        // create notify socket
        connectNotify();
    }

    /**
     * Sends one notify to the queue. This should be used to wake up the waiting thread.
     */
    void notify();

    /**
     * Clears one notify from the queue. Never try to clear more notifies than are available.
     */
    void clearNotify();

    /**
     * Block indefinitely until raw data is available or a notify is triggered.
     * If raw data remains in the socket or a notify was not cleared, wait will return immediately.
     * If a connection is not established, only notify will be waited on.
     * Note: EOF is also reported as readable
     *
     * @param readable True if data is available
     * @param notify True if notify was triggered
     * @return False on error
     */
    bool wait(bool &readable, bool &notify);

    /**
     * Reads from the socket and returns immediately.
     * Note: waitReadable does not guarantee that any application data will actually be available.
     *
     * @param buffer Target buffer (can be reused from previous readNonBlocking)
     * @param size Maximum number of bytes to read
     * @return 1 if data was read, 0 if no data was available, -1 if an error/disconnect occurred
     */
    int readNonBlocking(Buffer &buffer, uint32_t size);

    /**
     * Reads a protocol generated class from the connection
     *
     * @param buffer Reusable target buffer.
     * @param pgen the protocol generated class to be read from connection,
     * must have T::deserialize(const Buffer&, uint32_t &missing)
     * @return True on success
     */
    template <typename T>
    int readProtoClass(Buffer &buffer, T& pgen) {
        // try to deserialize, read missing bytes
        uint32_t missing = 0;
        while (!pgen.deserialize(buffer, missing)) {
            if (missing == 0) // no bytes missing, but class cannot be deserialized => error
                return -1;

            int read = readNonBlocking(buffer, missing);
            if (read <= 0)
                return read;
        }

        buffer.clear();
        return 1;
    }

    /* Forward everything not read related */

    using Connection::connect;
    using Connection::tryConnect;
    using Connection::disconnect;
    using Connection::connected;
    using Connection::protocol;
    using Connection::info;
    using Connection::socket;
    using Connection::write;
    using Connection::writeProtoClass;

protected:
    void connectNotify();

    // special socket used for thread-safe wake up of waitReadable()
    Socket_ref mNotify;
};

#endif //COMMONS_PUSHCONNECTION_H
