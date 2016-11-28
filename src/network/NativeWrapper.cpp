#include "NativeWrapper.h"

/**
 * One-Time initialization for winsock
 */
class OneTimeInit {
public:
    OneTimeInit() {
#ifdef __WIN32
        WSADATA w;
        if (WSAStartup(MAKEWORD(2,2), &w) != 0) {
            // TODO proper fatal shutdown
            return;
        }
#endif
    }

    ~OneTimeInit() {
#ifdef __WIN32
        WSACleanup();
#endif
    }
};
OneTimeInit go;

int ::NativeWrapper::getaddrinfo(const char *__name, const char *__service, const struct addrinfo *__req,
                                 struct addrinfo **__pai) {
    return ::getaddrinfo(__name, __service, __req, __pai);
}

int ::NativeWrapper::socket(int __domain, int __type, int __protocol) {
    return ::socket(__domain, __type, __protocol);
}

int ::NativeWrapper::connect(int __fd, const sockaddr *__addr, socklen_t __len) {
    return ::connect(__fd, __addr, __len);
}

int ::NativeWrapper::close(int __fd) {
#if defined(__WIN32)
    return ::closesocket(__fd);
#else
    return ::close(__fd);
#endif
}

ssize_t ::NativeWrapper::recv(int socket, void *buffer, size_t length) {
    return ::recv(socket, buffer, length, 0);
}

ssize_t::NativeWrapper::send(int socket, const void *buffer, size_t length) {
    return ::send(socket, buffer, length, 0);
}
