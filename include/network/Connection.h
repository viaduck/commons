/*
 * Copyright (C) 2015-2018 The ViaDuck Project
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

#include <cstdint>
#include <string>

#include <openssl/ssl.h>

#include <secure_memory/Buffer.h>
#include <network/CertificateStorage.h>

/* On Windows, socket descriptor is not an int but a #define for something else. Other OS do not know these #defines */
#if ! defined(__WIN32)
#define SOCKET int
#endif

/**
 * Platform independent TCP/SSL client
 */
class Connection {
public:
    /**
     * Connection result codes
     */
    enum class ConnectResult {
        UNKNOWN,                    /**< Unknown result **/
        ERROR_INTERNAL,             /**< Internal OpenSSL error **/
        ERROR_RESOLVE,              /**< Error resolving hostname **/
        ERROR_CONNECT,              /**< Error connecting to host **/
        ERROR_SSL_GENERAL,          /**< General SSL failure (e.g. protocol, ..) **/
        ERROR_SSL_VERIFY,           /**< Error verifying host certificate **/
        SUCCESS,                    /**< Successful connect **/
        ERROR_INVALID_CERTPATH      /**< Invalid system certificate path **/
    };

    /**
     * Connection state
     */
    enum class Status {
        CONNECTED,                  /**< Connected to host **/
        UNCONNECTED,                /**< Not connected to host **/
        UNKNOWN                     /**< No connection established yet **/
    };

    /**
     * Network layer protocol
     */
    enum class Protocol {
        UNSET,                      /**< Not yet connected **/
        IPv4,                       /**< Internet protocol 4 **/
        IPv6                        /**< Internet protocol 6 **/
    };

    /**
     * Creates a connection object with the specified target. Does not connect yet.
     * @param host Hostname or IP (both 4 and 6 are supported) to connect to
     * @param port TCP port
     * @param ssl Whether to use SSL or not
     * @param verifyEnable Whether to verify the SSL certificate or not. Ignored if ssl is false
     * @param certPath Path to system certificates directory. If left empty, no system certificates are used.
     * @param certStore CertificateStorage that holds allowed/denied certificates. Default: application-wide singleton
     * @param timeout Connect, receive and send timeout in milliseconds. 0 keeps OS default
     */
    Connection(std::string host, uint16_t port, bool ssl = true, bool verifyEnable = true, std::string certPath = "",
               CertificateStorage &certStore = CertificateStorage::getInstance(), uint32_t timeout = 0);

    /**
     * Creates a connection object with the specified target. Does not connect yet.
     * @param host Hostname or IP (both 4 and 6 are supported) to connect to
     * @param port TCP port
     * @param ssl Whether to use SSL or not
     * @param verifyEnable Whether to verify the SSL certificate or not. Ignored if ssl is false
     * @param timeout Connect, receive and send timeout in milliseconds. 0 keeps OS default
     */
    Connection(std::string host, uint16_t port, bool ssl, bool verifyEnable, uint32_t timeout) :
            Connection(host, port, ssl, verifyEnable, "", CertificateStorage::getInstance(), timeout) { }

    /**
     * Closes the connection and frees up allocated resources.
     */
    ~Connection();

    /**
     * Establish a connection
     * @return Connection status
     */
    ConnectResult connect();

    /**
     * Closes the connection (if connected)
     * @return Whether the connection was closed (true) or there is no active connection (false)
     */
    bool close();

    /**
     * @return Current connection status
     */
    Status status() const {
        return mStatus;
    }

    /**
     * @return Used IP protocol for connection
     */
    Protocol protocol() const {
        return mProtocol;
    }

    /**
     * @return Hostname
     */
    const std::string &host() const {
        return mHost;
    }

    /**
     * @return Port
     */
    uint16_t port() const {
        return mPort;
    }

    /**
     * @return Whether connection is using SSL
     */
    bool isSSL() const {
        return mUsesSSL;
    }

    /**
     * Read exactly size bytes from remote into the buffer. Blocks while waiting
     * @param buffer Buffer receiving the read data
     * @param size Exact count of bytes to read
     * @return True if exactly bytes have been read, false if not
     */
    bool read(Buffer &buffer, uint32_t size);

    /**
     * Write from buffer to remote
     * @param buffer Buffer to send to remote
     * @return Success (true) or not (false)
     */
    bool write(const Buffer &buffer);

    /**
     * Write a protocol generated class to the request stream
     * @param pclass the class to write, needs to have T::serialize(const Buffer&)
     *
     * @return True on success
     */
    template<typename T>
    bool writeProtoClass(const T &pclass) {
        Buffer outBuf;
        pclass.serialize(outBuf);
        return write(outBuf);
    }

    /**
     * Reads a protocol generated class from request stream
     * @param pclass the protocol generated class to be filled from buffer, needs to have T::deserialize(const Buffer&, uint32_t &missing)
     *
     * @return True on success
     */
    template<typename T>
    bool readProtoClass(T &pclass) {
        Buffer inBuf;
        uint32_t missing = 0;
        // try to deserialize, read missing bytes
        while(!pclass.deserialize(inBuf, missing)) {
            if(missing == 0) // no bytes missing, but class cannot be deserialized => error
                return false;

            if(!read(inBuf, missing))
                return false;
        }
        return true;
    }

protected:
    /**
     * Initializes the SSL connection
     * @return Result
     */
    ConnectResult initSsl();
    /**
     * Initializes certificate verification routines
     */
    bool initVerification();

    std::string mHost;
    uint16_t mPort;
    uint32_t mTimeout;
    bool mUsesSSL, mVerifyEnable;
    std::string mCertPath;
    CertificateStorage &mCertStore;
    Protocol mProtocol = Protocol::UNSET;
    Status mStatus = Status::UNKNOWN;

    //
    SOCKET mSocket;
    SSL *mSSL;
};

#endif //COMMONS_CONNECTION_H
