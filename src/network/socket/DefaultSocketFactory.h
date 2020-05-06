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

#ifndef COMMONS_DEFAULTSOCKETFACTORY_H
#define COMMONS_DEFAULTSOCKETFACTORY_H

#include <network/socket/ISocketFactory.h>

#include "SSLSocket.h"

class DefaultSocketFactory : public ISocketFactory {
public:
    ISocket *create(const ConnectionInfo &info) override {
        return info.ssl() ? new SSLSocket(info) : new TCPSocket(info);
    }
};

#endif //COMMONS_DEFAULTSOCKETFACTORY_H
