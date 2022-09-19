/*
 * Copyright (C) 2015-2021 The ViaDuck Project
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

#ifndef COMMONS_NATIVE_H
#define COMMONS_NATIVE_H

/* network includes */
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <wincrypt.h>
    #define NW__SHUT_RDWR SD_BOTH
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <fcntl.h>
    #include <dirent.h>
    #include <cerrno>
    #define SOCKET int
    #define INVALID_SOCKET  (~0)
    #define SOCKET_ERROR    (-1)
    #define NW__SHUT_RDWR SHUT_RDWR
#endif

#include <memory>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <commons/util/Except.h>
#include <commons/util/File.h>

#define _BEGIN_NATIVE_NAMESPACE namespace Native {
#define _END_NATIVE_NAMESPACE }

DEFINE_ERROR(native_init, base_error);

_BEGIN_NATIVE_NAMESPACE

/**
 * Platform one-time init
 */
class Init {
    using X509_ref = std::unique_ptr<X509, decltype(&X509_free)>;
public:
#ifdef WIN32
    Init() {
        // windows socket startup
        WSADATA w;
        L_assert(WSAStartup(MAKEWORD(2,2), &w) == 0, native_init_error);

        // populate openssl certificate store from windows cert store
        mRootStore = X509_STORE_new();
        populateStore("ROOT", mRootStore);
    }

    ~Init() {
        // ignore return value
        WSACleanup();
    }

    // adapted from https://stackoverflow.com/a/40046425/207861
    void populateStore(const char *winStore, X509_STORE *targetStore) {
        // open windows CA store
        HCERTSTORE store = CertOpenSystemStore(0, winStore);
        L_assert(store, native_init_error);

        // due to the structure of the loop, each cert context will be automatically freed by the next iteration
        for (PCCERT_CONTEXT winCert = nullptr; (winCert = CertEnumCertificatesInStore(store, winCert)); ) {
            // warning: always pass the ptr as local variable, never directly (d2i_X509 modifies the ptr)
            const unsigned char *encodedCert = winCert->pbCertEncoded;
            // warning: do NOT pass a reuse parameter (d2i_X509 expects it to be valid X509 and preserves its data)
            X509_ref target(d2i_X509(nullptr, &encodedCert, winCert->cbCertEncoded), &X509_free);

            if (target)
                L_expect(X509_STORE_add_cert(targetStore, target.get()) != 0);
        }

        // close store
        L_assert(CertCloseStore(store, 0), native_init_error);
    }

    void defaultStore(SSL_CTX *ctx) {
        // Windows has no certs without the root store
        L_expect(mRootStore);

        if (mRootStore) {
            // ensure our store does not get freed on ctx store replace
            X509_STORE_up_ref(mRootStore);
            SSL_CTX_set_cert_store(ctx, mRootStore);
        }
    }

protected:
    X509_STORE *mRootStore = nullptr;

#elif defined(__ANDROID__)
    Init() {
        // on Android, load all certificates in system location explicitly
        L_expect(mRootStore = X509_STORE_new());

        X509_LOOKUP *lookup = nullptr;
        L_expect(lookup = X509_STORE_add_lookup(mRootStore, X509_LOOKUP_file()));

        // manually add all system certificates as files to OpenSSL
        // TODO user certs
        for (const auto &file : File::find("/system/etc/security/cacerts", "")) {
            L_expect(X509_load_cert_file(lookup, file.c_str(), X509_FILETYPE_PEM) == 1);
        }

        OPENSSL_init_ssl(0, nullptr);
    }

    ~Init() {
        if (mRootStore)
            X509_STORE_free(mRootStore);
    }

    void defaultStore(SSL_CTX *ctx) {
        if (mRootStore) {
            // ensure our store does not get freed on ctx store replace
            X509_STORE_up_ref(mRootStore);
            SSL_CTX_set_cert_store(ctx, mRootStore);
        }
    }

protected:
    X509_STORE *mRootStore = nullptr;

#else

    Init() {
        // paths taken from https://serverfault.com/a/722646
        const char *checkDirs[] = {
        #ifdef __ANDROID__
            "/system/etc/security/cacerts", // Android
        #else
            "/etc/ssl/certs",               // Debian / Ubuntu / Gentoo / OpenSUSE / Arch
            "/etc/pki/tls/certs",           // Fedora/RHEL 6 / OpenELEC / CentOS
            "/etc/openssl/certs",           // NetBSD
        #endif
        };

        for (auto checkDir : checkDirs) {
            if (isValid(checkDir)) {
                mVerifyLocation = checkDir;
                break;
            }
        }

        if (mVerifyLocation.empty())
            Log::warn << "Default certificate path could not be found";

        OPENSSL_init_ssl(0, nullptr);
    }

    ~Init() = default;

    bool isValid(const char *dirPath) {
        using dir_ref = std::unique_ptr<DIR, decltype(&closeDir)>;
        dir_ref dir(opendir(dirPath), &closeDir);

        // check directory exists and has some content
        return dir && readdir(dir.get()) != nullptr;
    }

    void defaultStore(SSL_CTX *ctx) {
        #ifdef COMMONS_SYSTEM_OPENSSL
            SSL_CTX_set_default_verify_paths(ctx);
        #else
            L_expect(!mVerifyLocation.empty());
            SSL_CTX_load_verify_locations(ctx, nullptr, mVerifyLocation.c_str());
        #endif
    }

protected:
    static void closeDir(DIR *dir) {
        if (dir)
            closedir(dir);
    }

    std::string mVerifyLocation;
#endif
};

// Declare all native functions in the Native namespace

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

static Init gInit;

_END_NATIVE_NAMESPACE

#endif //COMMONS_NATIVE_H
