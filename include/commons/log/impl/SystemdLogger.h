#ifndef VDCOMMONS_SYSTEMDLOGGER_H
#define VDCOMMONS_SYSTEMDLOGGER_H

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
            case LogLevel::INVALID_ENUM_VALUE:
                return SystemdLogLevel::INVALID;
        }
    }
};

#endif //VDCOMMONS_SYSTEMDLOGGER_H
