#include <unistd.h>

#include <libCom/network/Connection.h>
#include <libCom/openssl_hook.h>
#include <openssl/ssl.h>

#include "NativeWrapper.h"

Connection::Connection(std::string host, uint16_t port, bool ssl) : mHost(host), mPort(port), mUsesSSL(ssl) {
    global_initOpenSSL();
}

Connection::~Connection() {
    close();
    // TODO cleanup?
}

Connection::ConnectResult Connection::connect() {
    struct addrinfo addressQuery;
    memset(&addressQuery, 0, sizeof(addressQuery));

    // address query parameters
    addressQuery.ai_family = PF_UNSPEC;
    addressQuery.ai_socktype = SOCK_STREAM;
    addressQuery.ai_protocol = IPPROTO_TCP;

    // resolve hostname
    struct addrinfo *addressInfoTmp = nullptr;
    int res = NativeWrapper::getaddrinfo(mHost.c_str(), std::to_string(mPort).c_str(), &addressQuery, &addressInfoTmp);

    std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> addressInfo(addressInfoTmp, &freeaddrinfo);

    if (res != 0)
        return ConnectResult::ERROR_RESOLVE;

    // iterate over all available addresses
    for (struct addrinfo *it = addressInfo.get(); it; it = it->ai_next) {
        mSocket = NativeWrapper::socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (mSocket == -1)
            return ConnectResult::ERROR_INTERNAL;

        // call global connect function
        res = NativeWrapper::connect(mSocket, it->ai_addr, it->ai_addrlen);
        if (res == -1) {
            NativeWrapper::close(mSocket);
            mSocket = INVALID_SOCKET;
        } else {
            switch (it->ai_family) {
                case AF_INET:
                    mProtocol = Protocol::IPv4; break;
                case AF_INET6:
                    mProtocol = Protocol::IPv6; break;
                default:
                    mProtocol = Protocol::UNSET;
            }

            // try to establish SSL session
            if (mUsesSSL && initSsl() != SSLResult::SUCCESS) {
                // TODO more verbose error reporting
                return ConnectResult::ERROR_SSL;
            }

            return ConnectResult::SUCCESS;
        }
    }

    // no resolved address has been successful
    // TODO more verbose error reporting
    return ConnectResult::ERROR_CONNECT;
}

bool Connection::close() {
    if (mSocket != SOCKET_ERROR) {
        NativeWrapper::close(mSocket);
        return true;
    }
    return false;
}

Connection::SSLResult Connection::initSsl() {
    const SSL_METHOD *method = TLS_client_method();
    if (method == nullptr) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    mSSLContext = SSL_CTX_new(method);
    if (mSSLContext == nullptr) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    // simplify application logic
    SSL_CTX_set_mode(mSSLContext, SSL_MODE_AUTO_RETRY);

    // TODO certificate verification
    mSSL = SSL_new(mSSLContext);
    if (mSSL == nullptr) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    if (SSL_set_fd(mSSL, mSocket) == 0) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    if (SSL_connect(mSSL) != 1) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_CONNECT;
    }

    return SSLResult::SUCCESS;
}
