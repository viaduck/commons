#ifndef LIBCOM_CONNECTION_H
#define LIBCOM_CONNECTION_H

#include <cstdint>
#include <string>
#include <openssl/ossl_typ.h>

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
