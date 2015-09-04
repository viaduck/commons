#ifndef __SOCKET_H_
#define __SOCKET_H_

#include <sys/socket.h>

#include "Buffer.h"

class Socket {
public:
    enum RESULT {
        RESULT_SUCCESS = 0,
        RESULT_FAILED,              // general error
        RESULT_ADDRLOOKUPFAILED,    // general error for getaddrinfo(..) fails
        RESULT_HOSTNOTFOUND,        // getaddrinfo(..): EAI_NONAME - hostname cannot be found
        RESULT_SOCKETCREATIONERROR, // socket cannot be created
        RESULT_CONNECTIONREFUSED,
        RESULT_CONNECTIONCLOSED,    // connection closed (recv returned -1)
        RESULT_SENDFAILED,
        RESULT_RECVFAILED,
    };

    Socket();
    ~Socket();


    RESULT init();
    RESULT connect(const char *host);
    void disconnect();
    RESULT recv(Buffer *buf);
    RESULT send(const Buffer &buf);

private:
    int mSocket;    // unix socket decriptor
    struct addrinfo *mHostInfo = nullptr;    // host info to connect to
};

#endif //__SOCKET_H_
