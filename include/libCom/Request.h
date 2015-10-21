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

// ensure windows compatibility
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

/**
 * Platform independent TCP-SSL client
 */
class Request {
protected:
	/**
	 * Hostname to connect to
	 */
	std::string host;

	/**
	 * Port to connect to
	 */
	u_short port;

	/**
	 * Use IPv6 (true) or IPv4 (false)
	 */
	bool ipv6;

	/**
	 * Initializes SSL for connection
	 * @param fd Socket descriptor
	 */
	int initSsl(int fd);

public:
	/**
	 * Creates a Request to host with port
	 * @param host Hostname to connect to
	 * @param port Port to connect to
	 * @param IPv6 Use IPv6 (true) or not (false; default)
	 */
	Request(std::string host, u_short port, bool IPv6=false);

	/**
	 * Free OpenSSL resources
	 */
	~Request();

	/**
	 * Connect to host
	 */
	int init();

	/**
	 * Read from remote into the buffer
	 * @param buffer Buffer receiving the read data
	 * @return Success (true) or not (false)
	 */
	bool read(Buffer &buffer);

    /**
	 * Read at most size bytes from remote into the buffer
	 * @param buffer Buffer receiving the read data
     * @param size Maximum bytes to read
	 * @return >0: Actual bytes read. 0: (clean) shutdown. <0: error occurred
	 */
	int32_t read(Buffer &buffer, const uint32_t size);

	/**
	 * Write from buffer to remote
	 * @param buffer Buffer to send to remote
	 * @return Success (true) or not (false)
	 */
	bool write(const Buffer &buffer);

private:
	SSL_CTX *ctx;
	SSL *ssl;
	bool initDone = false;
};


#endif //SOCKET_REQUEST_H
