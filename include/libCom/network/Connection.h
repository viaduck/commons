#ifndef LIBCOM_CONNECTION_H
#define LIBCOM_CONNECTION_H

#include <cstdint>
#include <string>
#include <openssl/ssl.h>

#include "libCom/Buffer.h"

/* On Windows, socket descriptor is not an int but a #define for something else. Other OS do not know these #defines */
#if ! defined(__WIN32)
#define SOCKET int
#endif

/**
 * Platform independent TCP/SSL client
 */
class Connection {
public:
    enum class ConnectResult {
        UNKNOWN, ERROR_INTERNAL, ERROR_RESOLVE, ERROR_CONNECT, ERROR_SSL, SUCCESS
    };

    enum class Status {
        CONNECTED, UNCONNECTED, UNKNOWN
    };

    enum class Protocol {
        UNSET,
        IPv4,
        IPv6
    };

    /**
     * Creates a connection object with the specified target. Does not connect yet.
     * @param host Hostname or IP (both 4 and 6 are supported) to connect to
     * @param port TCP port
     * @param ssl Whether to use SSL or not
     */
    Connection(std::string host, uint16_t port, bool ssl=true);

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
     * @return Wether the connection was closed (true) or there is no active connection (false)
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
	 * Read from remote into the buffer (greed - as max bytes as available)
	 * @param buffer Buffer receiving the read data
     * @param min Minimum size to read
	 * @return Success (true) or not (false)
	 */
    bool read(Buffer &buffer, const uint32_t min = 0);

    /**
	 * Read at most size bytes from remote into the buffer
	 * @param buffer Buffer receiving the read data
     * @param size Maximum bytes to read
	 * @return >0: Actual bytes read. 0: (clean) shutdown. <0: error occurred
	 */
    int32_t readMax(Buffer &buffer, const uint32_t size);

    /**
     * Read exactly size bytes from remote into the buffer
     * @param buffer Buffer receiving the read data
     * @param size Exact count of bytes to read
     * @return True if exactly bytes have been red, false if not
     */
    bool readExactly(Buffer &buffer, const uint32_t size);

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

            if(!readExactly(inBuf, missing))
                return false;
        }
        return true;
    }

protected:
    enum class SSLResult {
        ERROR_INTERNAL, ERROR_CONNECT, SUCCESS
    };
    SSLResult initSsl();

    std::string mHost;
    uint16_t mPort;
    bool mUsesSSL;
    Protocol mProtocol = Protocol::UNSET;
    Status mStatus = Status::UNKNOWN;

    //
    SOCKET mSocket;
    SSL_CTX *mSSLContext;
    SSL *mSSL;
};


#endif //LIBCOM_CONNECTION_H
