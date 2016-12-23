#include <libCom/log/Log.h>
#include "LogTest.h"

class StdoutLogger : public ILogger {
public:
    bool open() override {
        return true;
    }

    void close() override {

    }

    bool isOpen() override {
        return true;
    }

    std::ostream &stream(LogLevel level) override {
        std::cout<<"["<<level<<"] ";
        return std::cout;
    }

    std::ostream &stream() override {
        return std::cout;
    }
};

TEST_F(LogTest, Simple) {
    std::cout<<"Blub"<<std::endl;
    Log::get().registerLogger(new StdoutLogger());
    Log::dbg<<"Test 123 "<<456<<" "<<true<<"\n";
    Log::err<<"This is an error "<<1337<<" "<<0.0559897f<<"\n";
}