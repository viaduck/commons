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

#include <commons/log/Log.h>
#include <commons/log/impl/StdoutLogger.h>
#include "LogTest.h"

TEST_F(LogTest, Simple) {
    Log::dbg<<"Test 123 "<<456<<" "<<true;
    Log::err<<"This is an error "<<std::hex<<1337<<" "<<0.0559897f;

    EXPECT_EQ("[LogLevel::LEVEL_DEBUG] Test 123 456 1\n", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_EQ("[LogLevel::LEVEL_ERROR] This is an error 539 0.0559897\n", mLogger->toString(LogLevel::LEVEL_ERROR));
}

TEST_F(LogTest, StdOut) {
    StdoutLogger stdoutLogger;

    // just log to stdout for manual overview
    Log::get().unregisterLogger(mLogger);
    Log::get().registerLogger(&stdoutLogger);
    Log::dbg << "Test test 123";

    // enable mLogger again
    Log::get().unregisterLogger(&stdoutLogger);
    Log::get().registerLogger(mLogger);
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
    EXPECT_EQ("[LogLevel::LEVEL_DEBUG] Test 123\n", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_EQ("[LogLevel::LEVEL_ERROR] 1337 456\n", mLogger->toString(LogLevel::LEVEL_ERROR));
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
    EXPECT_NE("[LogLevel::LEVEL_DEBUG] Test 123\n", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_NE("[LogLevel::LEVEL_ERROR] 1337 456\n", mLogger->toString(LogLevel::LEVEL_ERROR));


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
    EXPECT_NE("[LogLevel::LEVEL_DEBUG] Test 123\n", mLogger->toString(LogLevel::LEVEL_DEBUG));
    EXPECT_EQ("[LogLevel::LEVEL_ERROR] 1337 456\n", mLogger->toString(LogLevel::LEVEL_ERROR));
}

TEST_F(LogTest, DefaultLogger) {
    // just log to stdout for manual overview
    Log::get().unregisterLogger(mLogger);

    auto loggers = Log::get().loggers();
    EXPECT_EQ(1u, loggers.size());
    EXPECT_EQ(&Log::get().defaultLogger(), loggers.front());

    Log::dbg << "Test test 123";
}
