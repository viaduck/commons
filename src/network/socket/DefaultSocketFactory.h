#ifndef CORE_DEFAULTSOCKETFACTORY_H
#define CORE_DEFAULTSOCKETFACTORY_H

#include <network/socket/ISocketFactory.h>

#include "SSLSocket.h"

class DefaultSocketFactory : public ISocketFactory {
public:
    ISocket *create(const ConnectionInfo &info) override {
        return info.ssl() ? new SSLSocket(info) : new TCPSocket(info);
    }
};

#endif //CORE_DEFAULTSOCKETFACTORY_H
