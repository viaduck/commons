#include "Socket.h"
#include <sys/types.h>
#include <sys/socket.h>
//#define _BSD_SOURCE
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#define SOCKET_BUF_SIZE 512

Socket::Socket()
{
}

Socket::~Socket() {
    disconnect();

    // TODO secure delete hostInfoList
    freeaddrinfo(mHostInfo);
}

Socket::RESULT Socket::init() {
    // TODO maybe hard-coding IP in code. Is this a bad idea?
    int ret;

    // lookup host information
    sockaddr_in addr;
    struct addrinfo hostInfo;
    struct addrinfo *hostInfoList;  // receives information about host to be looked up

    // as the documentation states, every field should be initialized to zero first
    memset(&hostInfo, 0, sizeof(addrinfo));

    hostInfo.ai_family = AF_INET;   // FIXME we use IPv4 for the moment
    hostInfo.ai_socktype = SOCK_STREAM; // tcp protocol

    // lookup the host
    ret = getaddrinfo("127.0.0.1", "1235", &hostInfo, &hostInfoList);
    if (ret != 0) {
        if (ret == EAI_NONAME)  // hostname cannot be looked up
            return RESULT_HOSTNOTFOUND;
        else    // other error
            return RESULT_ADDRLOOKUPFAILED;
    }

    // debug
    struct addrinfo *currentHostInfoList = hostInfoList;
    while (currentHostInfoList != nullptr) {
        char str[INET_ADDRSTRLEN];
        if (currentHostInfoList->ai_family == AF_INET &&
                currentHostInfoList->ai_addrlen > 0) {
            inet_ntop(AF_INET, &((struct sockaddr_in *)currentHostInfoList->ai_addr)->sin_addr, str, INET_ADDRSTRLEN);
            std::cout << "Addr: " << str << std::endl;
        }
        std::cout<<"Protocol: "<<currentHostInfoList->ai_protocol<<std::endl;
        // get next
        currentHostInfoList = currentHostInfoList->ai_next;
    }
    // debug end

    addr = *((struct sockaddr_in *)hostInfoList->ai_addr);
    mHostInfo = hostInfoList;

    // create a ipv4 tcp socket decriptor
    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket == -1)
        return RESULT_SOCKETCREATIONERROR;

    return RESULT_SUCCESS;
}

Socket::RESULT Socket::connect(const char *host) {
    if (mHostInfo == nullptr)
        return RESULT_FAILED;

    int ret;

    // connect
    ret = ::connect(mSocket, mHostInfo->ai_addr, mHostInfo->ai_addrlen);
    if (ret != 0) {
        return RESULT_CONNECTIONREFUSED;
    }

    return RESULT_SUCCESS;
}

void Socket::disconnect() {
    close(mSocket);
}

Socket::RESULT Socket::recv(Buffer *buf) {
    // TODO secure clean
    //uint8_t rbuf[SOCKET_BUF_SIZE];
    Buffer b(SOCKET_BUF_SIZE);
    ssize_t c = ::recv(mSocket, b.data(), SOCKET_BUF_SIZE, 0);
    if (c == -1)
        return errno == ECONNREFUSED ? RESULT_CONNECTIONCLOSED : RESULT_RECVFAILED;
    else if (c == 0)
        return RESULT_CONNECTIONCLOSED;
    else {
        buf->append(b.data(), c);
        return RESULT_SUCCESS;
    }
}

Socket::RESULT Socket::send(const Buffer &buf) {
    uint32_t n = buf.size();
    uint32_t p = 0;
    while (n > 0) {
        ssize_t c = ::send(mSocket, buf.const_data(p), n, 0);
        if (c == -1)
            return RESULT_SENDFAILED;
        n -= c;     // remove count of bytes we just sent
        p += c;     // move forward in Buffer
    }
    return RESULT_SUCCESS;
}
