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
     * @param ssl Whether to use SSL
     * @param sslVerify Whether to verify SSL hostname
     * @param certPath Path to system certificates. Leave empty for default system path.
     * @param certStore Custom certificate store
     * @param timeoutC Connect timeout in milliseconds. Default indefinite.
     * @param timeoutIO IO timout in milliseconds. Default indefinite.
     */
    ConnectionInfo(std::string host, uint16_t port, bool ssl = true, bool sslVerify = true, std::string certPath = "",
                   CertStore &certStore = CertStore::getInstance(), uint32_t timeoutC = 0, uint32_t timeoutIO = 0) :
                        mHost(std::move(host)), mPort(port),
                        mSSL(ssl), mSSLVerify(sslVerify),
                        mCertPath(std::move(certPath)), mCertStore(certStore),
                        mTimeoutConnect(timeoutC), mTimeoutIO(timeoutIO) {

    }

    const std::string &host() const {
        return mHost;
    }

    uint16_t port() const {
        return mPort;
    }

    uint32_t timeoutConnect() const {
        return mTimeoutConnect;
    }

    uint32_t timeoutIO() const {
        return mTimeoutIO;
    }

    bool ssl() const {
        return mSSL;
    }

    bool sslVerify() const {
        return mSSLVerify;
    }

    const std::string &certPath() const {
        return mCertPath;
    }

    const CertStore &certStore() const {
        return mCertStore;
    }

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
    CertStore &mCertStore;

    // timeouts
    uint32_t mTimeoutConnect;
    uint32_t mTimeoutIO;
};

#endif //COMMONS_CONNECTIONINFO_H
