/*
 * Copyright (C) 2019 The ViaDuck Project
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
#include <commons/thread/impl/LockFreeMessageQueue.h>
#include <commons/thread/impl/LockingMessageQueue.h>
#include <commons/thread/IQueueWorker.h>
#include "ThreadTest.h"

#define TEST_ITER 100

struct TestMessage {
    int testVal;
};

void testBasic(IMessageQueue<TestMessage> &queue) {
    for (int i = 0; i < TEST_ITER; i++)
        queue.push(new TestMessage{i});

    TestMessage *value;
    for (int i = 0; i < TEST_ITER; i++) {
        ASSERT_TRUE(queue.pop(value)) << i;
        EXPECT_EQ(i, value->testVal) << i;
        delete value;
    }
    ASSERT_FALSE(queue.pop(value));
}

void testAdvanced(IMessageQueue<TestMessage> &queue) {
    std::thread t([&queue] () {
        TestMessage *value;
        for (int i = 0; i < TEST_ITER; i++) {
            ASSERT_TRUE(queue.pop_wait(value)) << i;
            EXPECT_EQ(i, value->testVal) << i;
            delete value;
        }

        // infinite wait
        ASSERT_FALSE(queue.pop_wait(value));
    });

    for (int i = 0; i < TEST_ITER; i++)
        queue.push(new TestMessage{i});

    // let it wait, abort
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    queue.abort();

    // if wait does not abort, force gtest into timeout
    t.join();
}

void testBasicWorker(IQueueWorker<TestMessage> &worker) {
    worker.startThread();
    for (int i = 0; i < TEST_ITER; i++)
        worker.enqueue(new TestMessage{i});
    worker.stopThread();
}

void testAdvancedWorker(IQueueWorker<TestMessage> &worker) {
    worker.startThread();
    for (int i = 0; i < TEST_ITER / 2; i++)
        worker.enqueue(new TestMessage{i});

    // let it wait
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    for (int i = TEST_ITER / 2; i < TEST_ITER / 2; i++)
        worker.enqueue(new TestMessage{i});

    std::this_thread::sleep_for(100ms);
    worker.stopThread();
}

#define MAKE_QUEUE_TEST(test, impl)             \
    TEST_F(ThreadTest, test##impl) {            \
        impl##MessageQueue<TestMessage> queue;  \
        test(queue);                            \
    }

#define MAKE_IMPL_WORKER(impl) \
class impl##TestWorker : public IQueueWorker<TestMessage> { \
public: \
    impl##TestWorker() : IQueueWorker(new impl##MessageQueue<TestMessage>()) { } \
protected: \
    void doWork(const TestMessage *value) override { \
        ASSERT_EQ(mCounter++, value->testVal); \
    } \
    int mCounter = 0; \
};

#define MAKE_WORKER_TEST(test, impl) \
    TEST_F(ThreadTest, test##impl) {  \
        impl##TestWorker worker;            \
        test(worker);                       \
    }

#define MAKE_IMPL_TESTS(impl)               \
    MAKE_IMPL_WORKER(impl)                  \
    MAKE_QUEUE_TEST(testBasic, impl)        \
    MAKE_QUEUE_TEST(testAdvanced, impl)     \
    MAKE_WORKER_TEST(testBasicWorker, impl) \
    MAKE_WORKER_TEST(testAdvancedWorker, impl)

MAKE_IMPL_TESTS(Locking);
MAKE_IMPL_TESTS(LockFree);