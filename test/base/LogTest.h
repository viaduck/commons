#ifndef COMMONS_LOGTEST_H
#define COMMONS_LOGTEST_H

#include <gtest/gtest.h>

class LogTest : public ::testing::Test{
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
