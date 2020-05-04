/*
 * Copyright (C) 2020 The ViaDuck Project
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

#include "ThreadLocalTest.h"
#include <commons/thread/ThreadLocal.h>
#include <atomic>

TEST_F(ThreadLocalTest, testPrimitive) {
    ThreadLocal<int> test;

    // test throw on missing
    ASSERT_THROW(test.load(), std::out_of_range);

    // test basic store/load
    test.store(1);
    ASSERT_EQ(1, test.load());

    // test convenience
    test = 2;
    ASSERT_EQ(2, test.load());
}

TEST_F(ThreadLocalTest, testAdvanced) {
    ThreadLocal<std::string> test;

    // test throw on missing
    ASSERT_THROW(test.load(), std::out_of_range);

    // test basic store/load
    test.store("a");
    ASSERT_EQ("a", test.load());

    // test convenience
    test = "b";
    ASSERT_EQ("b", test.load());
}

class TestDummy { };

TEST_F(ThreadLocalTest, testComplex) {
    ThreadLocal<std::unique_ptr<TestDummy>> test;
    auto *test1 = new TestDummy(), *test2 = new TestDummy();
    std::unique_ptr<TestDummy> ptr1(test1), ptr2(test2);

    // test throw on missing
    ASSERT_THROW(test.load(), std::out_of_range);

    // test basic store/load
    test.store(std::move(ptr1));
    ASSERT_EQ(test1, test.load().get());

    // test convenience
    test = std::move(ptr2);
    ASSERT_EQ(test2, test.load().get());
}

void runOnThread(const std::function<void()> &cb) {
    std::atomic_bool flag(false);
    std::thread t([&] () {
        cb();
        flag.store(true);
    });

    while (!flag.load());
    t.join();
}

TEST_F(ThreadLocalTest, testThreading) {
    ThreadLocal<int> test;
    test = -1;
    ASSERT_EQ(-1, test.load());

    for (int i = 0; i < 25; i++)
        runOnThread([&] () {
            test = i;
            ASSERT_EQ(i, test.load());
        });

    ASSERT_EQ(-1, test.load());
}

TEST_F(ThreadLocalTest, testThreadingFactory) {
    ThreadLocal<int> test([] () { return 1; });
    test = -1;
    ASSERT_EQ(-1, test.load());

    for (int i = 0; i < 25; i++)
        runOnThread([&] () {
            ASSERT_EQ(1, test.load());
        });

    ASSERT_EQ(-1, test.load());
}
