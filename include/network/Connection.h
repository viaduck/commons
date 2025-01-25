/*
 * Copyright (C) 2015-2025 The ViaDuck Project
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

#include <network/component/NetworkResult.h>
#include <network/socket/ISocketFactory.h>

DEFINE_ERROR(connection, base_error);
DEFINE_ERROR(resolve, connection_error);

class Resolver;

/**
 * Platform independent TCP/SSL client
 */
class Connection {
public:
    using Ref = std::shared_ptr<Connection>;

    explicit Connection(ConnectionInfo connectionInfo);
    Connection(const std::string &host, uint16_t port, bool ssl = true);
    virtual ~Connection();

    /// @return Connection information
    [[nodiscard]] const ConnectionInfo &info() const {
        return mInfo;
    }

    /// @return Current connection status
    [[nodiscard]] bool connected() const;

    /// @return IP protocol used for connection
    [[nodiscard]] IPProtocol protocol() const;

    /// @return Underlying socket. Only valid when connected
    [[nodiscard]] ISocket *socket();

    /**
     * Set the socket factory used when establishing a connection
     *
     * @tparam T Socket factory
     */
    template<typename T>
    void setFactory() {
        mSocketFactory = std::make_unique<T>();
    }

    /**
     * Tries to establish a connection (non-blocking)
     *
     * Call this method again and again until the connection is established.
     * Not all socket types support non-blocking connect.
     */
    NetworkResult connectNonBlocking();
    /**
     * Establish a connection (blocking until timeout).
     *
     * @throws connection_error If no connection could be established
     */
    void connect();
    /**
     * Tries to establish a connection (blocking until timeout)
     *
     * @return True on success, false on error
     */
    bool tryConnect();

    /// Closes the connection if connected
    void disconnect();

    /**
     * Read exactly size bytes from connection (blocking until timeout)
     *
     * @param buffer Buffer receiving the read data
     * @param size Exact count of bytes to read
     * @return True if exactly size bytes have been read
     */
    bool read(Buffer &buffer, uint32_t size);
    /**
     * Read available bytes from the socket (non-blocking)
     *
     * @param buffer Target buffer, reuse this between calls for append/resume reading
     * @param size Maximum number of bytes to read
     */
    NetworkResult readNonBlocking(Buffer &buffer, uint32_t size);

    /**
     * Write to connection
     *
     * @param buffer Buffer to write
     * @return True on success
     */
    bool write(const Buffer &buffer);
    /**
     * Write a serializable class to the connection
     *
     * @param pgen the class to write, needs to have T::serialize(const Buffer&)
     * @return True if the serializable was successfully written
     */
    template<typename T>
    bool writeSerializable(const T &serializable) {
        Buffer outBuf;
        serializable.serialize(outBuf);
        return write(outBuf);
    }

    /**
     * Reads a serializable class from the connection (blocking until timeout)
     *
     * @param serializable the serializable class to be read from connection,
     * must have T::deserialize(const Buffer&, uint32_t &missing)
     * @return True if the serializable was successfully read
     */
    template<typename T>
    bool readSerializable(T &serializable) {
        Buffer inBuf;
        uint32_t missing = 0;
        // try to deserialize, read missing bytes
        while (!serializable.deserialize(inBuf, *&missing)) {
            // no bytes missing, but cannot be deserialized -> error
            if (missing == 0)
                return false;

            // read error
            if (!read(inBuf, missing))
                return false;
        }
        return true;
    }
    /**
     * Reads a serializable class from the connection (non-blocking)
     *
     * @param buffer Reusable target buffer.
     * @param serializable the protocol generated class to be read from connection,
     * must have T::deserialize(const Buffer&, uint32_t &missing)
     */
    template <typename T>
    NetworkResult readSerializableNonBlocking(Buffer &buffer, T& serializable) {
        // try to deserialize, read missing bytes non-blocking
        uint32_t missing = 0;
        while (!serializable.deserialize(buffer, *&missing)) {
            // no bytes missing, but cannot be deserialized -> error
            if (missing == 0)
                return NetworkInternalError();

            // read success, try to deserialize the next part of the serializable
            NetworkResult rv = readNonBlocking(buffer, missing);
            if (rv == NetworkResultType::SUCCESS)
                continue;

            // read deferred or error
            return rv;
        }

        // reset buffer for next read
        buffer.clear();
        return NetworkResultType::SUCCESS;
    }

protected:
    // constant connection information
    ConnectionInfo mInfo;
    // current connect flag
    bool mConnected = false;
    // current socket factory
    std::unique_ptr<ISocketFactory> mSocketFactory;
    // current socket
    std::unique_ptr<ISocket> mSocket;
    // internal hostname resolver
    std::unique_ptr<Resolver> mResolver;
};

#endif //COMMONS_CONNECTION_H
