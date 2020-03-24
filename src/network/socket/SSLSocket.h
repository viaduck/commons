/*
 * Copyright (C) 2019 The ViaDuck Project
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

#ifndef COMMONS_SSLSOCKET_H
#define COMMONS_SSLSOCKET_H

#include <openssl/err.h>

#include "TCPSocket.h"
#include "../ssl/SSLContext.h"

DEFINE_ERROR(ssl_socket, socket_error);
DEFINE_ERROR(ssl_verification, ssl_socket_error);

class SSLSocket : public TCPSocket {
    using SSL_ref = std::unique_ptr<SSL, decltype(&SSL_free)>;
public:
    explicit SSLSocket(const ConnectionInfo &info) : TCPSocket(info), mSSL(nullptr, &SSL_free) {}

    ~SSLSocket() override {
        if (mSSL) {
            // gracefully shut down ssl
            if (SSL_shutdown(mSSL.get()) == 0) {
                // shutdown not yet finished, require SSL_read for bidirectional shutdown
                read(nullptr, 0);
            }
        }
    }

    bool isReused() const {
        return SSL_session_reused(mSSL.get()) == 1;
    }

    bool connect(addrinfo *addr) override {
        return TCPSocket::connect(addr) && initSSL();
    }

    ssize_t read(void *data, uint32_t size) override {
        return SSL_read(mSSL.get(), data, size);
    }

    ssize_t write(const void *data, uint32_t size) override {
        return SSL_write(mSSL.get(), data, size);
    }

protected:
    static int verify_ssl_cert(int pre, X509_STORE_CTX *store) {
        X509 *cert = X509_STORE_CTX_get_current_cert(store);
        if (cert) {
            // get context user data
            SSLContext &ctx = SSLContext::getInstance();
            SSL *ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(store, SSL_get_ex_data_X509_STORE_CTX_idx()));
            auto *instance = static_cast<SSLSocket*>(SSL_get_ex_data(ssl, ctx.dataIndex()));

            if (instance->mInfo.sslVerify())
                return instance->mInfo.certStore().verify(pre == 1, X509_get0_pubkey(cert)) ? 1 : 0;

            return 1;
        }
        return 0;
    }

    static int saveSession(SSL *ssl, SSL_SESSION *session) {
        SSLContext &ctx = SSLContext::getInstance();
        auto *instance = static_cast<SSLSocket*>(SSL_get_ex_data(ssl, ctx.dataIndex()));

        // store session for resumption
        ctx.saveSession(instance->mInfo, session);

        // do not remove session
        return 1;
    }

    bool initSSL() {
        // load thread specific ssl context
        SSLContext &ctx = SSLContext::getInstance();
        ctx.load(mInfo.certPath());
        // register session resumption callbacks
        SSL_CTX_sess_set_new_cb(ctx.get(), saveSession);
        // set verification function
        SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_PEER, verify_ssl_cert);

        // create ssl object for this socket
        mSSL.reset(SSL_new(ctx.get()));
        L_assert(mSSL, ssl_socket_error);
        // set hostname for verification
        L_assert(SSL_set_tlsext_host_name(mSSL.get(), mInfo.host().c_str()) == 1, ssl_socket_error);
        // store this pointer to access later
        SSL_set_ex_data(mSSL.get(), ctx.dataIndex(), this);
        // pass socket to ssl
        L_assert(SSL_set_fd(mSSL.get(), mSocket) == 1, ssl_socket_error);

        // try to resume session
        SSL_SESSION *session = ctx.getSession(mInfo);
        if (session)
            L_assert(SSL_set_session(mSSL.get(), session) == 1, ssl_socket_error);

        // try to connect
        int ret = SSL_connect(mSSL.get());
        if (ret != 1 && SSL_get_error(mSSL.get(), ret) == SSL_ERROR_SSL &&
                ERR_GET_LIB(ERR_peek_last_error()) == ERR_LIB_SSL &&
                ERR_GET_REASON(ERR_peek_last_error()) == SSL_R_CERTIFICATE_VERIFY_FAILED)
            throw ssl_verification_error("Certificate verification failed");
        else if (ret != 1)
            throw ssl_socket_error("SSL connection failed");

#ifdef TLS1_3_VERSION
        // TLSv1.3: recommends that each SSL_SESSION object only used once
        if (SSL_version(mSSL.get()) == TLS1_3_VERSION)
            ctx.removeSession(mInfo, session);
#endif

        // only for chaining in connect
        return true;
    }

    // internal ssl object, not thread-safe
    SSL_ref mSSL;
};

#endif //COMMONS_SSLSOCKET_H
