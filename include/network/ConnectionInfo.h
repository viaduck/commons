/*
 * Copyright (C) 2019-2023 The ViaDuck Project
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

#ifndef COMMONS_CONNECTIONINFO_H
#define COMMONS_CONNECTIONINFO_H

#include <network/ssl/CertStore.h>

class ConnectionInfo {
public:
    /**
     * Constructs a ConnectionInfo
     *
     * @param host IP address or hostname
     * @param port Port number
     * @param ssl Whether to use SSL, defaults to true
     * @param sslVerify Whether to verify SSL hostname, defaults to true
     * @param certPath Path to system certificates. Leave empty for default system path.
     * @param certStore Custom certificate store, defaults to the global store
     * @param timeoutConnect Connect timeout in milliseconds. Default indefinite.
     * @param timeoutIO IO timeout in milliseconds. Default indefinite.
     */
    explicit ConnectionInfo(std::string host, uint16_t port, bool ssl = true, bool sslVerify = true,
                            std::string certPath = "", CertStore *certStore = CertStore::getInstance(),
                            uint32_t timeoutConnect = 0, uint32_t timeoutIO = 0)
            : mHost(std::move(host)), mPort(port), mSSL(ssl), mSSLVerify(sslVerify),
            mCertPath(std::move(certPath)), mCertStore(certStore),
            mTimeoutConnect(timeoutConnect), mTimeoutIO(timeoutIO) {

    }

    const std::string &host() const { return mHost; }
    void host(const std::string &value) { mHost = value; }

    uint16_t port() const { return mPort; }
    void port(uint16_t value) { mPort = value; }

    bool ssl() const { return mSSL; }
    void ssl(bool value) { mSSL = value; }

    bool sslVerify() const { return mSSLVerify; }
    void sslVerify(bool value) { mSSLVerify = value; }

    const std::string &certPath() const { return mCertPath; }
    void certPath(const std::string &value) { mCertPath = value; }

    const CertStore *certStore() const { return mCertStore; }
    CertStore *certStore() { return mCertStore; }
    void certStore(CertStore *value) { mCertStore = value; }

    uint32_t timeoutConnect() const { return mTimeoutConnect; }
    void timeoutConnect(uint32_t value) { mTimeoutConnect = value; }

    uint32_t timeoutIO() const { return mTimeoutIO; }
    void timeoutIO(uint32_t value) { mTimeoutIO = value; }

    size_t hash() const {
        return (std::hash<std::string>()(mHost) + 0x9e3779b9) ^ std::hash<uint16_t>()(mPort);
    }

private:
    // basic
    std::string mHost;
    uint16_t mPort;

    // ssl stuff
    bool mSSL;
    bool mSSLVerify;
    std::string mCertPath;
    CertStore *mCertStore;

    // timeouts
    uint32_t mTimeoutConnect;
    uint32_t mTimeoutIO;
};

#endif //COMMONS_CONNECTIONINFO_H
