/*
 * Copyright (C) 2023 The ViaDuck Project
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

#include <network/component/ConnectionInfo.h>
#include <network/component/Uri.h>

ConnectionInfo ConnectionInfo::parseConnectURI(const std::string &uriStr, uint16_t defaultPort) {
    // example: vd://net/?h=abc.com&p=1234&s=false
    Uri uri(uriStr);

    // check scheme and host
    L_assert_eq("vd", uri.schema(), parse_error);
    L_assert_eq("net", uri.host(), parse_error);

    // mandatory host
    auto host = uri.queryValue("h", "");
    L_assert(!host.empty(), parse_error);

    // optional port (fallback to defaultPort)
    auto portInt = std::stoi(uri.queryValue("p", "0"));
    if (portInt == 0) portInt = defaultPort;
    L_assert(portInt > 0 && portInt <= std::numeric_limits<uint16_t>::max(), parse_error);
    auto port = static_cast<uint16_t>(portInt);

    // optional use ssl (fallback to true)
    auto sslStr = uri.queryValue("s", "true");
    L_assert(sslStr == "true" || sslStr == "false", parse_error);
    auto ssl = (sslStr == "true");

    return ConnectionInfo(host, port, ssl);
}

bool ConnectionInfo::empty() const {
    return mHost.empty() && mPort == 0;
}
