#include <commons/log/Log.h>
#include "LogTest.h"

TEST_F(LogTest, Simple) {
    Log::dbg<<"Test 123 "<<456<<" "<<true<<"\n";
    Log::err<<"This is an error "<<std::hex<<1337<<" "<<0.0559897f<<"\n";

    EXPECT_EQ("[LogLevel::DEBUG] Test 123 456 1\n", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_EQ("[LogLevel::ERROR] This is an error 539 0.0559897\n", mLogger->toString(LogLevel::LEVEL_ERROR));
}

TEST_F(LogTest, EnableDisable) {
    // check default enabled status
    EXPECT_TRUE(Log::get().isEnabled());
    EXPECT_TRUE(Log::dbg.isEnabled());
    EXPECT_TRUE(Log::trac.isEnabled());
    EXPECT_TRUE(Log::info.isEnabled());
    EXPECT_TRUE(Log::warn.isEnabled());
    EXPECT_TRUE(Log::err.isEnabled());

    // check logging functionality if enabled
    Log::dbg<<"Test 123";
    Log::err<<"1337 456";
    EXPECT_EQ("[LogLevel::DEBUG] Test 123", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_EQ("[LogLevel::ERROR] 1337 456", mLogger->toString(LogLevel::LEVEL_ERROR));
    mLogger->clear();

    // disable logging globally
    Log::get().disableLogLevel();
    EXPECT_FALSE(Log::get().isEnabled());
    EXPECT_FALSE(Log::dbg.isEnabled());
    EXPECT_FALSE(Log::trac.isEnabled());
    EXPECT_FALSE(Log::info.isEnabled());
    EXPECT_FALSE(Log::warn.isEnabled());
    EXPECT_FALSE(Log::err.isEnabled());
    Log::dbg<<"Test 123";
    Log::err<<"1337 456";
    EXPECT_NE("[LogLevel::DEBUG] Test 123", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_NE("[LogLevel::ERROR] 1337 456", mLogger->toString(LogLevel::LEVEL_ERROR));


    // disable individual log level
    Log::get().enableLogLevel();
    Log::dbg.setEnabled(false);
    EXPECT_TRUE(Log::get().isEnabled());
    EXPECT_FALSE(Log::dbg.isEnabled());
    EXPECT_TRUE(Log::trac.isEnabled());
    EXPECT_TRUE(Log::info.isEnabled());
    EXPECT_TRUE(Log::warn.isEnabled());
    EXPECT_TRUE(Log::err.isEnabled());
    Log::dbg<<"Test 123";
    Log::err<<"1337 456";
    EXPECT_NE("[LogLevel::DEBUG] Test 123", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_EQ("[LogLevel::ERROR] 1337 456", mLogger->toString(LogLevel::LEVEL_ERROR));
}