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

#include "commons/log/Log.h"

#include <algorithm>

void Log::registerLogger(ILogger *logger) {
    if (logger->isOpen() || logger->open())
        mLoggers.push_back(logger);
}

void Log::unregisterLogger(ILogger *logger) {
    if (logger->isOpen())
        logger->close();
    mLoggers.erase(std::remove(mLoggers.begin(), mLoggers.end(), logger), mLoggers.end());
}

Log::LogStream<LogLevel::LEVEL_TRACE> Log::trac(Log::mInstance);
Log::LogStream<LogLevel::LEVEL_DEBUG> Log::dbg(Log::mInstance);
Log::LogStream<LogLevel::LEVEL_INFO> Log::info(Log::mInstance);
Log::LogStream<LogLevel::LEVEL_WARNING> Log::warn(Log::mInstance);
Log::LogStream<LogLevel::LEVEL_ERROR> Log::err(Log::mInstance);
Log Log::mInstance;
