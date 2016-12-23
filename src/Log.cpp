#include "libCom/log/Log.h"

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

LogStream<LogLevel::TRACE> Log::trac(Log::mInstance);
LogStream<LogLevel::DEBUG> Log::dbg(Log::mInstance);
LogStream<LogLevel::INFO> Log::info(Log::mInstance);
LogStream<LogLevel::WARNING> Log::warn(Log::mInstance);
LogStream<LogLevel::ERROR> Log::err(Log::mInstance);
Log Log::mInstance;
