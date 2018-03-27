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
