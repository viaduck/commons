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

#ifndef COMMONS_LOG_H
#define COMMONS_LOG_H

#include <commons/log/ILogger.h>
#include <commons/log/impl/StdoutLogger.h>

#include <vector>
#include <ostream>
#include <mutex>

/**
 * General purpose logging class. ILoggers can be registered to direct different log levels to different outputs.
 */
class Log {
    /**
     * Provides a wrapper for std::ostream that streams all logged values to Log's registered loggers.
     * @tparam Level Assigned LogLevel
     */
    template <LogLevel Level>
    class LogStream {
        /**
         * Chained stream operator calls will use the following class, that logs only if a ILogger declared it wants to log
         * the current stream.
         */
        class LogStreamValue {
        public:
            /**
             * Constructor that accepts the parent LogStream and a vector of enabled ILoggers for this log stream.
             * @param parent
             * @param enabledLoggers
             */
            LogStreamValue(LogStream &parent, std::vector<ILogger*> enabledLoggers) : mParent(parent),
                                                                                      mEnabledLoggers(std::move(enabledLoggers)) { }
            /**
             * Appends a std::endl to the log entry on destruction.
             */
            ~LogStreamValue() {
                if (mParent.mLog.isEnabled() && mParent.isEnabled()) {
                    for (ILogger *logger: mEnabledLoggers) {
                        if (logger->isOpen())
                            logger->flush();
                    }

                    mParent.mLog.mLogLock.unlock();
                }
            }

            /**
             * Stream operator which is used for logging. This only logs the actual value by passing them to LogStream.
             *
             * Logging is only performed if the log level is enabled.
             * @tparam T Type of value to be logged
             * @param t Value to log
             */
            template<typename T>
            LogStreamValue &operator<<(const T &t) {
                if (mParent.mLog.isEnabled() && mParent.isEnabled()) {
                    for (ILogger *logger: mEnabledLoggers) {
                        if (logger->isOpen())
                            logger->stream() << t;
                    }
                }
                return *this;
            }

        protected:
            LogStream &mParent;
            std::vector<ILogger*> mEnabledLoggers;
        };

    public:
        /**
         * Stream operator which is used for logging. This redirects all logged values to the registered ILoggers.
         * The subsequent values will be logged by returned LogStreamValue.
         * @tparam T Type of value to be logged
         * @param t Value to log
         * @return Chained log stream, that accepts all other values
         */
        template<typename T>
        LogStreamValue operator<<(const T &t) {
            std::vector<ILogger *> enabledLoggers;

            if (mLog.isEnabled() && mEnabled) {
                mLog.mLogLock.lock();

                enabledLoggers.reserve(mLog.loggers().size());
                for (ILogger *logger: mLog.loggers()) {
                    // only log if logger wants this level
                    if (logger->wantsLog(Level)) {
                        // add to list of enabled loggers
                        enabledLoggers.push_back(logger);

                        // log the first value if it's open
                        if (logger->isOpen())
                            logger->stream() << t;
                    }
                }
            }
            return LogStreamValue(*this, enabledLoggers);
        }

        /**
         * @return Whether this log level should be enabled. This respects global enable.
         */
        bool isEnabled() const {
            return mLog.isEnabled() && mEnabled;
        }
        /**
         * Enables or disabled this log level.
         * @param enabled True if log level should be enabled, false if not.
         */
        void setEnabled(bool enabled) {
            mEnabled = enabled;
        }

    protected:
        /**
         * @param log Global Log
         */
        explicit LogStream(Log &log) : mLog(log) { }

        Log &mLog;
        bool mEnabled = true;

        friend class Log;
    };

public:
    /**
     * Registers an ILogger to the Log. The ILogger will be opened (if it's not already), if this fails, the ILogger
     * won't be added!
     * @param logger ILogger to register
     */
    void registerLogger(ILogger *logger);

    /**
     * Unregisters an ILogger from the Log. The ILogger will be closed (if it's not already).
     * @param logger ILogger to unregister
     */
    void unregisterLogger(ILogger *logger);

    /**
     * Disables only a certain LogLevel.
     * @param level If this is set to LogLevel::VALUE_INVALID, logging is completely disabled, which has priority
     *        over individual enabled status of log levels.
     */
    void disableLogLevel(LogLevel level = LogLevel::VALUE_INVALID) {
        switch (level) {
            case LogLevel::LEVEL_TRACE: trac.setEnabled(false); break;
            case LogLevel::LEVEL_DEBUG: dbg.setEnabled(false); break;
            case LogLevel::LEVEL_INFO: info.setEnabled(false); break;
            case LogLevel::LEVEL_WARNING: warn.setEnabled(false); break;
            case LogLevel::LEVEL_ERROR: err.setEnabled(false); break;
            case LogLevel::VALUE_INVALID: mEnabled = false; break;
        }
    }
    /**
     * Enables only a certain LogLevel.
     * @param level If this is set to LogLevel::VALUE_INVALID, logging is completely enabled, which has priority
     *              over individual enabled status of log levels.
     */
    void enableLogLevel(LogLevel level = LogLevel::VALUE_INVALID) {
        switch (level) {
            case LogLevel::LEVEL_TRACE: trac.setEnabled(true); break;
            case LogLevel::LEVEL_DEBUG: dbg.setEnabled(true); break;
            case LogLevel::LEVEL_INFO: info.setEnabled(true); break;
            case LogLevel::LEVEL_WARNING: warn.setEnabled(true); break;
            case LogLevel::LEVEL_ERROR: err.setEnabled(true); break;
            case LogLevel::VALUE_INVALID: mEnabled = true; break;
        }
    }
    /**
     * Sets a certain LogLevel. All levels below will be disabled, all levels above enabled.
     * @param level If this is set to LogLevel::INVALID_ENUM_VALUE, logging is completely disabled for each individual
     *              log levels as well as the global level.
     */
     void setLogLevel(LogLevel level) {
         bool found;
         trac.setEnabled(found = (level == LogLevel::LEVEL_TRACE));
         dbg.setEnabled(found = (found || level == LogLevel::LEVEL_DEBUG));
         info.setEnabled(found = (found || level == LogLevel::LEVEL_INFO));
         warn.setEnabled(found = (found || level == LogLevel::LEVEL_WARNING));
         err.setEnabled(found = (found || level == LogLevel::LEVEL_ERROR));
         mEnabled = found;
     }

    /**
     * @return Global log enabled status, which has priority over individual enabled status of log levels.
     */
    bool isEnabled() const {
        return mEnabled;
    }
    void setEnabled(bool value = true) {
        mEnabled = value;
    }

    /**
     * @return Currently registered loggers. Returns at least the default logger (see Log::defaultLogger())
     */
    std::vector<ILogger *> loggers() {
        if (mLoggers.empty())
            return {&mDefaultLogger};
        return mLoggers;
    }

    StdoutLogger &defaultLogger() {
        return mDefaultLogger;
    }

    /**
     * Trace log level
     */
    static LogStream<LogLevel::LEVEL_TRACE> trac;
    /**
     * Debug log level
     */
    static LogStream<LogLevel::LEVEL_DEBUG> dbg;
    /**
     * Info log level
     */
    static LogStream<LogLevel::LEVEL_INFO> info;
    /**
     * Warning log level
     */
    static LogStream<LogLevel::LEVEL_WARNING> warn;
    /**
     * Error log level
     */
    static LogStream<LogLevel::LEVEL_ERROR> err;

    /**
     * @return Singleton Log instance
     */
    static Log &get() {
        return mInstance;
    }
protected:
    explicit Log() { }

    std::vector<ILogger *> mLoggers;

    bool mEnabled = true;
    StdoutLogger mDefaultLogger;
    std::mutex mLogLock;

    static Log mInstance;

    template <LogLevel Level>
    friend class LogStream;
};

#endif //COMMONS_LOG_H
