#include <libCom/log/Log.h>
#include "LogTest.h"

TEST_F(LogTest, Simple) {
    Log::get().registerLogger(mLogger);

    Log::dbg<<"Test 123 "<<456<<" "<<true<<"\n";
    Log::err<<"This is an error "<<std::hex<<1337<<" "<<0.0559897f<<"\n";

    EXPECT_EQ("[LogLevel::DEBUG] Test 123 456 1\n", mLogger->toString(0));
    EXPECT_EQ("[LogLevel::ERROR] This is an error 539 0.0559897\n", mLogger->toString(1));
}
