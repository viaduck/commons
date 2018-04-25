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

#include <unistd.h>

#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/x509_vfy.h>

#include <network/Connection.h>
#include <network/ConnectionInfo.h>
#include <network/SSLContext.h>

#include "NativeWrapper.h"

// global one-time network init on a per-platform basis
NativeInit gNativeInit;

Connection::Connection(std::string host, uint16_t port, bool ssl, bool verifyEnable, std::string certPath,
                       CertificateStorage &certStore, uint32_t timeout) :
        mHost(host), mPort(port), mTimeout(timeout), mUsesSSL(ssl), mVerifyEnable(verifyEnable),
        mCertPath(certPath), mCertStore(certStore), mSocket(INVALID_SOCKET) {

}

Connection::~Connection() {
    close();
}

/**
 * Sets a socket's send and receive timeouts.
 * @param s Socket descriptor
 * @param t Timeout in milliseconds
 */
void socket_io_timeout(SOCKET s, uint32_t t) {
    if (t == 0)
        return;

#ifdef WIN32
    DWORD tv = t;
#else
    timeval tv = { .tv_sec = 0, .tv_usec = 1000 * t };
#endif

    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
}

/**
 * Sets a socket's non-blocking state.
 * @param s Socket descriptor
 * @param value True for non-blocking state, false for blocking state
 */
void socket_io_nonblock(SOCKET s, bool value) {
#ifdef WIN32
    u_long mode = value ? 1 : 0;  // 1 to enable non-blocking socket
    ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, NULL);
    if (value)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    fcntl(s, F_SETFL, flags);
#endif
}

/**
 * Handles special DNS resolved addresses
 *
 * Currently handled:
 * - 127.0.53.53: DNS name collision (https://www.icann.org/news/announcement-2-2014-08-01-en)
 * @param address Input address
 * @return False indicates a failure, true success
 */
bool handle_special_DNS(const addrinfo *addr) {
    const static uint32_t DNS_NAME_COLLISION = ntoh(static_cast<uint32_t>(0x7f003535));       // 127.0.53.53

    // IPv4
    if (addr->ai_family == AF_INET) {
        // see https://www.icann.org/news/announcement-2-2014-08-01-en
        if (reinterpret_cast<sockaddr_in *>(addr->ai_addr)->sin_addr.s_addr == DNS_NAME_COLLISION)
            return false;
    }
    return true;
}

/**
 * Resolves a host-port combination into addrinfo structs.
 *
 * @param host Host
 * @param port Port
 * @param cb Callback that receives addrinfo structs. It can be called multiple times. A return value of true
 *           instructs the resolve method to stop resolving addresses.
 * @return False if there were no addresses resolved or no callback returned true. True otherwise.
 */
bool resolve(const char *host, const char *port, const std::function<bool(const addrinfo&)> &cb) {
    addrinfo addressQuery;
    memset(&addressQuery, 0, sizeof(addressQuery));

    // address query parameters
    addressQuery.ai_family = PF_UNSPEC;
    addressQuery.ai_socktype = SOCK_STREAM;
    addressQuery.ai_protocol = IPPROTO_TCP;

    // resolve hostname
    addrinfo *addressInfoTmp = nullptr;
    if (NativeWrapper::getaddrinfo(host, port, &addressQuery, &addressInfoTmp) == 0) {
        std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> addressInfo(addressInfoTmp, &NativeWrapper::freeaddrinfo);

        // iterate over all available addresses
        for (const addrinfo *it = addressInfo.get(); it && handle_special_DNS(it); it = it->ai_next) {
            // wait for callback to indicate success
            if (cb(*it))
                return true;
        }
    }

    return false;
}

/**
 * Tries connecting to host described by addr.
 *
 * @param addr Host info
 * @param timeout Connection timeout
 * @param sock Receives Socket descriptor on success, INVALID_SOCKET if socket couldn't be created
 * @return True if connection was successful. False otherwise.
 */
bool try_connect(const addrinfo &addr, uint32_t timeout, SOCKET &sock) {
    // create socket
    sock = NativeWrapper::socket(addr.ai_family, addr.ai_socktype, addr.ai_protocol);

    if (sock != INVALID_SOCKET) {
        // set read / write timeout
        socket_io_timeout(sock, timeout);
        // make socket non blocking
        socket_io_nonblock(sock, true);

        // call global connect function, return immediately because of no block
        int res = NativeWrapper::connect(sock, addr.ai_addr, addr.ai_addrlen);

#ifdef WIN32
        if (res == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        if (res == -1 && errno == EINPROGRESS) {
#endif
            // create a set of sockets for select, add only our socket
            fd_set set;
            FD_ZERO(&set);
            FD_SET(sock, &set);

            // timeout -> do not pass ZERO, instead pass NULL to block
            timeval *ptv = nullptr;
            if (timeout > 0) {
                timeval tv = {.tv_sec = 0, .tv_usec = 1000 * timeout};
                ptv = &tv;
            }

            // waits for socket to complete connect (become writeable)
            if (NativeWrapper::select(sock + 1, nullptr, &set, nullptr, ptv) > 0) {
                // connect completed - successfully or not

                int error;
                socklen_t len = sizeof(error);
                if (NativeWrapper::getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len) == 0 && error == 0) {
                    // connect success

                    // make socket blocking again
                    socket_io_nonblock(sock, false);
                    return true;
                }
            }
        }

        // timeout or error
        NativeWrapper::close(sock);
    }

    return false;
}

Connection::ConnectResult Connection::connect() {
    ConnectResult res;
    bool canResolve = resolve(mHost.c_str(), std::to_string(mPort).c_str(), [&] (const addrinfo &addr) -> bool {
        bool canConnect = try_connect(addr, mTimeout, mSocket);

        if (canConnect) {
            // connect success!

            switch (addr.ai_family) {
                case AF_INET:
                    mProtocol = Protocol::IPv4; break;
                case AF_INET6:
                    mProtocol = Protocol::IPv6; break;
                default:
                    mProtocol = Protocol::UNSET;
            }

            // connected only if no SSL requested or if SSL success
            if (!mUsesSSL || (res = initSsl()) == ConnectResult::SUCCESS) {
                mStatus = Status::CONNECTED;
                res = ConnectResult::SUCCESS;
            }
        }
        else if(mSocket == INVALID_SOCKET) {
            // socket error

            res = ConnectResult::ERROR_CONNECT;
        }
        else {
            // connect error -> ignore and try next addr
            return false;
        }

        // success, SSL error or socket error -> abort
        return true;
    });

    // TODO more verbose error reporting
    if (!canResolve)
        return ConnectResult::ERROR_RESOLVE;

    return res;
}

bool Connection::close() {
    if (mSocket != INVALID_SOCKET) {
        if (mUsesSSL && status() == Status::CONNECTED) {
            // save current SSL session
            SSLContext::getInstance().saveSession(*this, SSL_get1_session(mSSL));

            // disconnect & cleanup SSL
            SSL_shutdown(mSSL);
            SSL_free(mSSL);
        }

        NativeWrapper::shutdown(mSocket, NW__SHUT_RDWR);
        NativeWrapper::close(mSocket);
        mStatus = Status::UNCONNECTED;
        return true;
    }
    return false;
}

bool Connection::read(Buffer &buffer, uint32_t size) {
    if (status() != Status::CONNECTED)
        return false;

    uint32_t read = 0;
    ssize_t res = 1;
    buffer.increase(size, true);        // must be big enough to hold at least size bytes

    while (read != size && (res > 0)) {
        if (mUsesSSL)
            res = SSL_read(mSSL, buffer.data(buffer.size()), size-read);
        else
            res = NativeWrapper::recv(mSocket, buffer.data(buffer.size()), size-read);

        if (res > 0) {
            read += res;
            buffer.use(static_cast<uint32_t>(res));
        }
    }

    return read == size;
}

bool Connection::write(const Buffer &buffer) {
    if (status() != Status::CONNECTED)
        return false;

    ssize_t res;
    if (mUsesSSL)
        res = SSL_write(mSSL, buffer.const_data(), buffer.size());
    else
        res = NativeWrapper::send(mSocket, buffer.const_data(), buffer.size());

    if (res <= 0)
        return false;

    uint32_t writtenbytes = static_cast<uint32_t>(res);
    return writtenbytes == buffer.size();
}

Connection::ConnectResult Connection::initSsl() {
    if (!initVerification())  {
        // TODO more verbose error reporting
        return ConnectResult::ERROR_INVALID_CERTPATH;
    }

    mSSL = SSL_new(SSLContext::getInstance());
    if (mSSL == nullptr) {
        // TODO more verbose error reporting
        return ConnectResult::ERROR_INTERNAL;
    }

    SSL_set_tlsext_host_name(mSSL, mHost.c_str());
    // storing this to access CertificateStorage member
    SSL_set_ex_data(mSSL, CertificateStorage::getOpenSSLDataIndex(), this);

    if (SSL_set_fd(mSSL, mSocket) == 0) {
        // TODO more verbose error reporting
        return ConnectResult::ERROR_INTERNAL;
    }

    // set stored session
    SSL_SESSION *session = SSLContext::getInstance().getSession(*this);
    if (session != nullptr)
        SSL_set_session(mSSL, session);

    int ret;
    if ((ret = SSL_connect(mSSL)) != 1) {
        switch (SSL_get_error(mSSL, ret)) {
            case SSL_ERROR_SSL: {
                unsigned long err = ERR_peek_last_error();
                int lib = ERR_GET_LIB(err);
                int reason = ERR_GET_REASON(err);

                if (lib == ERR_LIB_SSL && reason == SSL_R_CERTIFICATE_VERIFY_FAILED)
                    return ConnectResult::ERROR_SSL_VERIFY;
                else
                    return ConnectResult::ERROR_SSL_GENERAL;
            }
            case SSL_ERROR_NONE:
                return ConnectResult::ERROR_CONNECT;
            default:
                return ConnectResult::ERROR_SSL_GENERAL;
        }
    }

    // count how often we saved the user's traffic
    if (SSL_session_reused(mSSL))
        SSLContext::getInstance().mSessionsResumed++;

    return ConnectResult::SUCCESS;
}

bool Connection::initVerification() {
    if (mCertPath.empty()) {
        // load platform specific default certificate store
        gNativeInit.defaultStore(SSLContext::getInstance());
    }
    else if (SSL_CTX_load_verify_locations(SSLContext::getInstance(), nullptr, mCertPath.c_str()) == 0) {
        // try to load given cert path
        return false;
    }

    SSL_CTX_set_verify(SSLContext::getInstance(), SSL_VERIFY_PEER, [] (int preVerify, X509_STORE_CTX *ctx) -> int {
        // preVerify indicates success of last level. 1 is success, 0 is fail
        // return value: 1 is success, 0 is fail

        X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
        if (cert) {
            EVP_PKEY *pubKey = X509_get0_pubkey(cert);

            // get context user data
            SSL *ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
            Connection *thiz = static_cast<Connection*>(SSL_get_ex_data(ssl, CertificateStorage::getOpenSSLDataIndex()));

            // allow any certificate if verification is turned off
            if (!thiz->mVerifyEnable)
                return 1;

            switch (thiz->mCertStore.check(pubKey, CertificateStorage::Mode::UNDECIDED)) {
                case CertificateStorage::Mode::DENY:
                    return 0;
                case CertificateStorage::Mode::ALLOW:
                    return 1;
                case CertificateStorage::Mode::UNDECIDED:
                    return preVerify;
            }
        }
        return 0;
    });
    return true;
}
