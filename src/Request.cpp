#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <libCom/Buffer.h>
#include <libCom/BufferRange.h>
#include <libCom/openssl_hook.h>
#include "libCom/Request.h"

using namespace std;

Request::Request(std::string host, u_short port, bool IPv6) {
	this->host = host;
	this->port = port;
	this->ipv6 = IPv6;

	global_initOpenSSL();
}

Request::~Request() {
    if (initDone) {
        SSL_CTX_free(ctx);
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
}

void printError() {
    std::cerr << ERR_func_error_string(ERR_get_error());
}

int Request::initSsl(int fd) {
	const SSL_METHOD *method;

	method = TLSv1_2_client_method();
	if (method == nullptr) {
		return -3;
	}

	ctx = SSL_CTX_new(method);
	if (ctx == nullptr) {
        printError();
        return -4;
    }

	ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        printError();
        return -5;
    }
	if (SSL_set_fd(ssl, fd) == 0) {
        printError();
        return -6;
    }
    if (SSL_connect(ssl) != 1) {
        printError();
        return -7;
    }

    initDone = true;
    
    return 0;
}

int Request::init() {
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd == SOCKET_ERROR) {
        return -1;
    }

    sockaddr_in service;
    if (this->ipv6) {
        service.sin_port = AF_INET6;
    } else {
        service.sin_family = AF_INET;
    }
    service.sin_port = htons(this->port);
    service.sin_addr.s_addr = inet_addr(this->host.c_str());

    int result = connect(fd, reinterpret_cast<sockaddr *>(&service), sizeof(service));
    if (result == -1) {
        return -2;
    }

    return initSsl(fd);
}

bool Request::read(Buffer &buffer, const uint32_t min) {
    if (!initDone)
        return false;

    uint32_t read = 0;
    int res;
    uint8_t iters = 0;
    buffer.increase(buffer.size() + 512 * 4);     // must be big enough to hold at least 512 bytes (*4 for 4 iterations)

    // TODO read timeout, non-blocking?
	while ((res = SSL_read(ssl, buffer.data(buffer.size()), 512)) > 0) {
		buffer.use(static_cast<uint32_t>(res));
        read += res;

        if (res != 512 || read >= min)
            break;

        iters++;
        if (iters == 4) {      // buffer is not big enough for another iteration -> increase it (another 4 iterations)
            buffer.increase(buffer.size() + 512 * 4);
            iters = 0;
        }
	}

	return res >= 0;
}

int32_t Request::readMax(Buffer &buffer, const uint32_t size) {
    if (!initDone)
        return -1;

    int res;
    buffer.increase(size, true);     // must be big enough to hold at least size bytes

    // TODO read timeout, non-blocking?
    res = SSL_read(ssl, buffer.data(buffer.size()), size);
    if (res > 0)
        buffer.use(static_cast<uint32_t>(res));

    return res;
}

bool Request::readExactly(Buffer &buffer, const uint32_t size) {
    if (!initDone)
        return false;

    uint32_t read = 0;
    int res;
    buffer.increase(size, true);        // must be big enough to hold at least size bytes

    // TODO read timeout
    while ((res = SSL_read(ssl, buffer.data(buffer.size()), size-read)) > 0 && read != size) {
        read += res;
        buffer.use(static_cast<uint32_t>(res));
    }

    return read == size;
}

bool Request::write(const Buffer &buffer) {
    int res = SSL_write(ssl, buffer.const_data(), buffer.size());
    if(res < 0) return false;

    uint32_t writtenbytes = static_cast<uint32_t>(res);
    return initDone && writtenbytes == buffer.size();
}
