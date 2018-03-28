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

#ifndef COMMONS_SSLSESSION_H
#define COMMONS_SSLSESSION_H

#include <string>

#include <openssl/ssl.h>
#include <network/Connection.h>

/**
 * Storage class to store connection details: host and port
 */
class ConnectionInfo {
public:
    /**
     * Constructs a ConnectionInfo with the given connection details
     * @param host Hostname
     * @param port Port
     */
    ConnectionInfo(const std::string &host, uint16_t port) : mHost(host), mPort(port) { }

    /**
     * Constructs a ConnectionInfo from an existing Connection object
     * @param connection Existing Connection instance, host and port are copied
     */
    ConnectionInfo(const Connection &connection) : mHost(connection.host()), mPort(connection.port()) { }

    /**
     * Comparison operator needed for std::unordered_map
     * @param rhs Other object
     * @return True if both host and port are equal, false if not
     */
    bool operator==(const ConnectionInfo &rhs) const {
        return mHost == rhs.mHost && mPort == rhs.mPort;
    }

    /**
     * Comparison operator needed for std::unordered_map
     * @param rhs Other object
     * @return True if any of host or port differs, false if not
     */
    bool operator!=(const ConnectionInfo &rhs) const {
        return !(rhs == *this);
    }

    /**
     * @return Hostname
     */
    const std::string &host() const {
        return mHost;
    }

    /**
     * @return Port
     */
    uint16_t port() const {
        return mPort;
    }

protected:
    std::string mHost;
    uint16_t mPort;
};

namespace std {

    /**
     * Implement hash function for SSLSession class (so that SSLSession is usable in a hash map)
     */
    template<>
    struct hash<ConnectionInfo> {
        /**
        * Helper method for std::hash<> specializations to combine a proper hash value
        * @tparam T Type of value to hash
        * @param seed Current hash value, will be updated
        * @param v Value to hash
        */
        template <class T>
        inline void hash_combine(std::size_t& seed, const T& v) const
        {
            std::hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }

        std::size_t operator()(const ConnectionInfo &k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            size_t current = 0;
            hash_combine(current, hash<string>()(k.host()));
            hash_combine(current, hash<uint16_t>()(k.port()));
            return current;
        }
    };
}

#endif //COMMONS_SSLSESSION_H
