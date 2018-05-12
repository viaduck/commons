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

#ifndef COMMONS_LOGTEST_H
#define COMMONS_LOGTEST_H

#include <gtest/gtest.h>

class LogTest : public ::testing::Test {
protected:
    class TestLogger : public ILogger {
    public:
        TestLogger() : current(&bruce2) {}

        bool open() override {
            return true;
        }

        void close() override {

        }

        bool isOpen() override {
            return true;
        }

        bool wantsLog(LogLevel level) override {
            switch (level) {
                case LogLevel::LEVEL_DEBUG:
                    current = &bruce; break;
                case LogLevel::LEVEL_ERROR:
                    current = &bruce2; break;
                default:
                    break;
            }
            *current<<"["<<level<<"] ";
            return true;
        }

        std::ostream &stream() override {
            return *current;
        }

        std::string toString(LogLevel level) {
            switch(level) {
                case LogLevel::LEVEL_DEBUG: return bruce.str();
                case LogLevel::LEVEL_ERROR: return bruce2.str();
                default:
                    return current->str();
            }
        }

        void clear() {
            bruce.str(std::string());
            bruce.clear();

            bruce2.str(std::string());
            bruce2.clear();

            current = &bruce2;
        }

    protected:
        /**
         * Debug
         */
        std::stringstream bruce;
        /**
         * Error
         */
        std::stringstream bruce2;
        std::stringstream *current;
    };


    void SetUp() override {
        mLogger = new TestLogger();
        Log::get().registerLogger(mLogger);
    }

    void TearDown() override {
        Log::get().unregisterLogger(mLogger);
        delete mLogger;
    }

    TestLogger *mLogger;
};


#endif //COMMONS_LOGTEST_H
