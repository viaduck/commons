#ifndef LIBCOM_CONNECTION_H
#define LIBCOM_CONNECTION_H

#include <cstdint>
#include <string>

#include "libCom/Buffer.h"

/* network includes */
#if defined(__WIN32)
    #if defined(CONNECTION_TEST)
        #include "winsock_mock.h"
    #else
        #include <winsock2.h>
        #include <ws2tcpip.h>
    #endif
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif

/* On Windows, socket descriptor is not an int but a #define for something else. Other OS do not know these #defines */
#ifndef SOCKET_ERROR
#define SOCKET int
#define SOCKET_ERROR (-1)
#endif

/**
 * Platform independent TCP/SSL client
 */
class Connection {
public:
    enum class ConnectResult {
        UNKNOWN, ERROR_INTERNAL, ERROR_RESOLVE, ERROR_CONNECT, SUCCESS
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
     */
    Connection(std::string host, uint16_t port);

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
    std::string mHost;
    uint16_t mPort;
    Protocol mProtocol = Protocol::UNSET;
    Status mStatus = Status::UNKNOWN;

    //
    SOCKET mSocket;
};


#endif //LIBCOM_CONNECTION_H
