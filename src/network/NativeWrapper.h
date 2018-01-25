#ifndef VDCOMMONS_NATIVEWRAPPER_H
#define VDCOMMONS_NATIVEWRAPPER_H

/* network includes */
#if defined(__WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <cerrno>
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

namespace NativeWrapper {
    int getaddrinfo (const char * __name, const char * __service,
                     const struct addrinfo * __req, struct addrinfo ** __pai);

    int socket(int __domain, int __type, int __protocol);

    int connect(int __fd, const sockaddr *__addr, socklen_t __len);

    int close(int __fd);

    ssize_t recv(int socket, void *buffer, size_t length);

    ssize_t send(int socket, const void *buffer, size_t length);

    void freeaddrinfo(struct addrinfo *__ai);

    int select(int ndfs, fd_set *_read, fd_set *_write, fd_set *_except, timeval *timeout);

    int getsockopt(int sockfd, int level, int optname, char *optval, socklen_t *optlen);
}

#endif //VDCOMMONS_NATIVEWRAPPER_H
