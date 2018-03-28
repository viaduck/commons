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

#ifndef COMMONS_STDOUTLOGGER_H
#define COMMONS_STDOUTLOGGER_H

#include <commons/log/Log.h>
#include <ctime>
#include <iomanip>

/**
 * This ILogger implementation logs all log levels to stdout.
 */
class StdoutLogger : public ILogger {
public:
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
        std::time_t now = std::time(nullptr);
        std::cout<<std::put_time(std::localtime(&now), "%FT%T%z")<<" ["<<level<<"] ";
        return true;
    }
};

#endif //COMMONS_STDOUTLOGGER_H
