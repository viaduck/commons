//
// Created by John Watson on 02.11.2016.
//

#ifndef CORE_WINSOCK_MOCK_H
#define CORE_WINSOCK_MOCK_H

    // used only for fixing redeclaration problems on windows
    #ifdef __WIN32__
        // used only on windows mingw64 to disable declaration of socket() functions so we can redeclare
        #define __WINSOCK_WS1_SHARED 1
        #include <winsock2.h>

        typedef int socklen_t;
        #define EAI_AGAIN WSATRY_AGAIN
        #define EAI_BADFLAGS WSAEINVAL
        #define EAI_FAIL WSANO_RECOVERY
        #define EAI_FAMILY WSAEAFNOSUPPORT
        #define EAI_MEMORY WSA_NOT_ENOUGH_MEMORY

        #define EAI_NONAME WSAHOST_NOT_FOUND
        #define EAI_SERVICE WSATYPE_NOT_FOUND
        #define EAI_SOCKTYPE WSAESOCKTNOSUPPORT

        #define EAI_NODATA 11004 /* WSANO_DATA */
        WINSOCK_API_LINKAGE u_long WSAAPI ntohl(u_long netlong);
        WINSOCK_API_LINKAGE u_short WSAAPI ntohs(u_short netshort);
        WINSOCK_API_LINKAGE u_long WSAAPI htonl(u_long hostlong);
        WINSOCK_API_LINKAGE u_short WSAAPI htons(u_short hostshort);
        typedef struct addrinfo {
            int ai_flags;
            int ai_family;
            int ai_socktype;
            int ai_protocol;
            size_t ai_addrlen;
            char *ai_canonname;
            struct sockaddr *ai_addr;
            struct addrinfo *ai_next;
        } ADDRINFOA,*PADDRINFOA;

        // let's mock the socket functions to emulate network behavior
        extern "C" {
            extern int getaddrinfo (const char *__restrict __name,
                             const char *__restrict __service,
                             const struct addrinfo *__restrict __req,
                             struct addrinfo **__restrict __pai);

            extern int socket(int __domain, int __type, int __protocol);

            extern int connect(int __fd, const sockaddr *__addr, socklen_t __len);

            extern int close(int __fd);
        }
    #endif

#endif //CORE_WINSOCK_MOCK_H
