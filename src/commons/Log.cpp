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

Log::LogStream<LogLevel::TRACE> Log::trac(Log::mInstance);
Log::LogStream<LogLevel::DEBUG> Log::dbg(Log::mInstance);
Log::LogStream<LogLevel::INFO> Log::info(Log::mInstance);
Log::LogStream<LogLevel::WARNING> Log::warn(Log::mInstance);
Log::LogStream<LogLevel::ERROR> Log::err(Log::mInstance);
Log Log::mInstance;
