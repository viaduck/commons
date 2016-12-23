#ifndef LIBCOM_LOG_H
#define LIBCOM_LOG_H

#include <logger/LogLevel.h>

#include <vector>
#include <ostream>

/**
 * Interface a Logger has to implement.
 * The internal ostream will get LogLevel as the first streamed value. Next values are logged values.
 */
class ILogger {
public:
    /**
     * Opens the output stream. This enables logging.
     * @return Whether opening succeeded.
     */
    virtual bool open() = 0;

    /**
     * Closes the output stream. No logging is possible after calling this function until the ILogger is opened again.
     */
    virtual void close() = 0;

    /**
     * @return Whether the ILogger is ready for logging
     */
    virtual bool isOpen() = 0;

    /**
     * @param level LogLevel
     * @return Underlying writable std::ostream stream.
     */
    virtual std::ostream &stream(LogLevel level) = 0;

    /**
     * @return Underlying writable std::ostream stream.
     */
    virtual std::ostream &stream() = 0;
};

template <LogLevel Level>
class LogStream;

/**
 * General purpose logging class. ILoggers can be registered to direct different log levels to different outputs.
 */
class Log {
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
     * Trace log level
     */
    static LogStream<LogLevel::TRACE> trac;
    /**
     * Debug log level
     */
    static LogStream<LogLevel::DEBUG> dbg;
    /**
     * Info log level
     */
    static LogStream<LogLevel::INFO> info;
    /**
     * Warning log level
     */
    static LogStream<LogLevel::WARNING> warn;
    /**
     * Error log level
     */
    static LogStream<LogLevel::ERROR> err;

    /**
     * @return Singleton Log instance
     */
    static Log &get() {
        return mInstance;
    }

protected:
    Log() { }

    std::vector<ILogger *> mLoggers;
    static Log mInstance;

    template <LogLevel Level>
    friend class LogStream;
};

/**
 * Provides a wrapper for std::ostream that streams all logged values to Log's registered loggers.
 * @tparam Level Assigned LogLevel
 */
template <LogLevel Level>
class LogStream : public std::ostream {
    /**
     * Chained stream operator calls will use the following class, that instructs the LogStream not to log the LogLevel
     * again.
     */
    class LogStreamValue : public std::ostream {
    public:
        LogStreamValue(LogStream &parent) : mParent(parent) { }
        /**
         * Stream operator which is used for logging. This only logs the actual value by passing them to LogStream.
         * @tparam T Type of value to be logged
         * @param t Value to log
         */
        template<typename T>
        LogStreamValue &operator<<(const T &t) {
            mParent.logValue(t);
            return *this;
        }

    protected:
        LogStream &mParent;
    };

public:
    /**
     * Stream operator which is used for logging. This redirects all logged values to the registered ILoggers.
     * The first redirected value is the assigned LogLevel, followed by the actual values.
     * @tparam T Type of value to be logged
     * @param t Value to log
     */
    template<typename T>
    LogStreamValue &operator<<(const T &t) {
        logValue(t, true);
        return mChild;
    }

protected:
    /**
     * Internal helper for logging. This redirects all logged values to the registered ILoggers.
     * @tparam T Type of value to be logged
     * @param t Vale to log
     * @param streamLevel Whether to stream the LogLevel or not
     */
    template<typename T>
    void logValue(const T &t, bool streamLevel = false) {
        for (ILogger *logger: mLog.mLoggers) {
            if (logger->isOpen()) {
                if (streamLevel)
                    logger->stream(Level);
                else
                    logger->stream() << t;
            }
        }
    }

    /**
     * Internal constructor that instantiates the value-only LogStream
     * @param log Global Log
     */
    LogStream(Log &log) : mLog(log), mChild(*this) { }

    Log &mLog;
    LogStreamValue mChild;

    friend class Log;
};

#endif //LIBCOM_LOG_H
