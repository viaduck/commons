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

#ifndef COMMONS_NATIVEWRAPPER_H
#define COMMONS_NATIVEWRAPPER_H

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

    void freeaddrinfo(struct addrinfo *__ai);

    int socket(int __domain, int __type, int __protocol);

    int shutdown(int __fd, int how);

    int close(int __fd);

    int getsockopt(int __fd, int level, int optname, char *optval, socklen_t *optlen);

    int connect(int __fd, const sockaddr *__addr, socklen_t __len);

    int select(int ndfs, fd_set *_read, fd_set *_write, fd_set *_except, timeval *timeout);

    ssize_t recv(int __fd, void *buffer, size_t length);

    ssize_t send(int __fd, const void *buffer, size_t length);
}

#endif //COMMONS_NATIVEWRAPPER_H
