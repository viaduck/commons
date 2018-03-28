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

#ifndef COMMONS_SYSTEMDLOGGER_H
#define COMMONS_SYSTEMDLOGGER_H

#include <commons/log/Log.h>
#include <ctime>
#include <iomanip>

/**
 * This ILogger implementation logs all log levels to stdout in systemd's special log level format.
 */
class SystemdLogger : public ILogger {
public:
    enum class SystemdLogLevel : int {
        EMERG = 0,
        ALERT = 1,
        CRIT = 2,
        ERR = 3,
        WARNING = 4,
        NOTICE = 5,
        INFO = 6,
        DEBUG = 7,
        INVALID = -1,
    };

    bool open() override {
        return true;
    }

    void close() override {}

    bool isOpen() override {
        return true;
    }

    std::ostream &stream() override {
        return std::cout;
    }

    bool wantsLog(LogLevel level) override {
        std::cout<<"<"<<std::to_string(static_cast<std::underlying_type<SystemdLogLevel>::type>(toSystemdLevel(level)))<<">";
        return true;
    }

    SystemdLogLevel toSystemdLevel(LogLevel level) {
        switch (level) {
            case LogLevel::LEVEL_TRACE:
                return SystemdLogLevel::DEBUG;
            case LogLevel::LEVEL_DEBUG:
                return SystemdLogLevel::DEBUG;
            case LogLevel::LEVEL_INFO:
                return SystemdLogLevel::INFO;
            case LogLevel::LEVEL_WARNING:
                return SystemdLogLevel::WARNING;
            case LogLevel::LEVEL_ERROR:
                return SystemdLogLevel::ERR;
            default:
                return SystemdLogLevel::INVALID;
        }
    }
};

#endif //COMMONS_SYSTEMDLOGGER_H
