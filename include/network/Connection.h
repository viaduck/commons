/*
 * Copyright (C) 2015-2019 The ViaDuck Project
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

#ifndef COMMONS_CONNECTION_H
#define COMMONS_CONNECTION_H

#include <secure_memory/Buffer.h>
#include <commons/util/Except.h>

#include <network/socket/ISocketFactory.h>

DEFINE_ERROR(connection, base_error);

/**
 * Platform independent TCP/SSL client
 */
class Connection {
public:
    explicit Connection(ConnectionInfo connectionInfo);

    Connection(const std::string &host, uint16_t port, bool ssl = true) : Connection(ConnectionInfo(host, port, ssl)) {}

    /**
     * Establish a connection
     */
    void connect();

    /**
     * Tries to establish a connection
     *
     * @return True on success
     */
    bool tryConnect();

    /**
     * Closes the connection if connected
     */
    void disconnect();

    /**
    * @return Current connection status
    */
    bool connected() const {
        return !!mSocket;
    }

    /**
     * @return IP protocol used for connection
     */
    IPProtocol protocol() const {
        return connected() ? mSocket->protocol() : IPProtocol::INVALID_ENUM_VALUE;
    }

    /**
     * @return Connection information
     */
    const ConnectionInfo &info() const {
        return mInfo;
    }

    /**
     * @return Underlying socket. Only valid while connected
     */
    ISocket *socket() {
        return mSocket.get();
    }

    /**
     * Set the socket factory
     * @tparam T Socket factory
     */
    template<typename T>
    void setFactory() {
        mSocketFactory = std::make_unique<T>();
    }

    /**
     * Read exactly size bytes from connection into buffer. Blocks while waiting
     *
     * @param buffer Buffer receiving the read data
     * @param size Exact count of bytes to read
     * @return True if exactly size bytes have been read
     */
    bool read(Buffer &buffer, uint32_t size);

    /**
     * Write buffer to connection
     *
     * @param buffer Buffer to write
     * @return True on success
     */
    bool write(const Buffer &buffer);

    /**
     * Write a protocol generated class to the connection
     *
     * @param pgen the class to write, needs to have T::serialize(const Buffer&)
     * @return True on success
     */
    template<typename T>
    bool writeProtoClass(const T &pgen) {
        Buffer outBuf;
        pgen.serialize(outBuf);
        return write(outBuf);
    }

    /**
     * Reads a protocol generated class from the connection
     *
     * @param pgen the protocol generated class to be read from connection,
     * must have T::deserialize(const Buffer&, uint32_t &missing)
     * @return True on success
     */
    template<typename T>
    bool readProtoClass(T &pgen) {
        Buffer inBuf;
        uint32_t missing = 0;
        // try to deserialize, read missing bytes
        while (!pgen.deserialize(inBuf, missing)) {
            if (missing == 0) // no bytes missing, but class cannot be deserialized => error
                return false;

            if (!read(inBuf, missing))
                return false;
        }
        return true;
    }

protected:
    using Socket_ref = std::unique_ptr<ISocket>;
    using SocketFactory_ref = std::unique_ptr<ISocketFactory>;

    // constant connection information
    ConnectionInfo mInfo;

    // current socket
    SocketFactory_ref mSocketFactory;
    Socket_ref mSocket;
};

#endif //COMMONS_CONNECTION_H
