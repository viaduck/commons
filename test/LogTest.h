#ifndef LIBCOM_LOGTEST_H
#define LIBCOM_LOGTEST_H

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
            if (current == &bruce)
                current = &bruce2;
            else
                current = &bruce;
            *current<<"["<<level<<"] ";
            return true;
        }

        std::ostream &stream() override {
            return *current;
        }

        std::string toString(int i) {
            switch(i) {
                case 0: return bruce.str();
                case 1: return bruce2.str();
                default:
                    return current->str();
            }
        }

    protected:
        std::stringstream bruce;
        std::stringstream bruce2;
        std::stringstream *current;
    };


    void SetUp() override {
        mLogger = new TestLogger();
    }

    void TearDown() override {
        mLogger->close();
        delete mLogger;
    }

    TestLogger *mLogger;
};


#endif //LIBCOM_LOGTEST_H
