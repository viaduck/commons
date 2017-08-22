#ifndef VDCOMMONS_SSLSESSION_H
#define VDCOMMONS_SSLSESSION_H

#include <string>

#include <openssl/ssl.h>
#include <commons/helper.h>
#include <commons/network/Connection.h>

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

#endif //VDCOMMONS_SSLSESSION_H
