/*
 * Copyright (C) 2018 The ViaDuck Project
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

#ifndef COMMONS_ILOGGER_H
#define COMMONS_ILOGGER_H

#include <enum/logger/LogLevel.h>

/**
 * Interface a Logger has to implement.
 * For every log stream (a chain of << calls) wantsLog(LogLevel) is called. If it returns true, logged values will be
 * passed to ILogger implementation.
 */
class ILogger {
public:
    /**
     * Virtual destructor
     */
    virtual ~ILogger() = default;

    /**
     * Opens the output stream. This enables logging.
     * @return Whether opening succeeded.
     */
    virtual bool open() {
        return true;
    }

    /**
     * Closes the output stream. No logging is possible after calling this function until the ILogger is opened again.
     */
    virtual void close() { }

    /**
     * @return Whether the ILogger is ready for logging.
     */
    virtual bool isOpen() {
        return true;
    }

    /**
     * Flushes the stream at end of log operation
     */
    virtual void flush() {
        stream() << std::endl;
    }

    /**
     * @return Underlying writable std::ostream stream.
     */
    virtual std::ostream &stream() = 0;

    /**
     * @param level LogLevel
     * @return Whether ILogger implementation wants to log stream started by this log level.
     */
    virtual bool wantsLog(LogLevel level) {
        return true;
    }
};

#endif //COMMONS_ILOGGER_H
