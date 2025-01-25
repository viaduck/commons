/*
 * Copyright (C) 2025 The ViaDuck Project
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

#ifndef COMMONS_NETWORKERROR_H
#define COMMONS_NETWORKERROR_H

// general base class for all network errors
class INetworkError {
public:
    virtual ~INetworkError() = default;
};

// common base class of all internal network errors (i.e. errors caused by internal issues)
class NetworkInternalError : public INetworkError { };
// internal: network/wait timeout error
class NetworkTimeoutError : public NetworkInternalError { };
// internal: some action is not supported on the socket
class NetworkUnsupportedSocketError : public NetworkInternalError { };
// internal: none of the resolved addresses were connectable
class NetworkNotConnectableError : public NetworkInternalError { };

// base of all SSL errors, as external network errors (i.e. errors caused by some external reason)
class NetworkSSLError : public INetworkError { };
// SSL: verification failed
class NetworkSSLVerificationError : public NetworkSSLError { };

// OS network error carrying the OS-specific errno/WSAGetLastError code
class NetworkOSError : public INetworkError {
public:
    explicit NetworkOSError();
    explicit NetworkOSError(int code) : mCode(code) { }

    [[nodiscard]] int code() const {
        return mCode;
    }
    void code(int value) {
        mCode = value;
    }

protected:
    int mCode;
};

// OS: resolve error carrying the error code
class NetworkResolveError : public NetworkOSError {
public:
    explicit NetworkResolveError(int code) : NetworkOSError(code) { }
};

#endif //COMMONS_NETWORKERROR_H
