#ifndef CORE_ISOCKETFACTORY_H
#define CORE_ISOCKETFACTORY_H

#include <network/socket/ISocket.h>
#include <network/ConnectionInfo.h>

class ISocketFactory {
public:
    virtual ~ISocketFactory() = default;

    /**
     * Creates instance of socket implementation as specified in connection info
     */
    virtual ISocket *create(const ConnectionInfo &info) = 0;
};

#endif //CORE_ISOCKETFACTORY_H
