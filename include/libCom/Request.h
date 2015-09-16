#ifndef SOCKET_REQUEST_H
#define SOCKET_REQUEST_H

#include <string>
#include <sstream>

#ifdef __WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif

#include "libCom/Buffer.h"

#include <openssl/ssl.h>


class Request {
protected:
	std::string host;
	u_short port;
	bool ipv6;

	int initSsl(int fd);

public:
	Request(std::string host, u_short port, bool IPv6=false);
	~Request();

	int init();
	bool read(Buffer &buffer);
	bool write(const Buffer &buffer);

private:
	SSL_CTX *ctx;
	SSL *ssl;
	bool initDone = false;
};


#endif //SOCKET_REQUEST_H
