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

#include "ValidPtrTest.h"

#include <commons/ValidPtr.h>

class TestImplementation : public ValidObject {
public:
    TestImplementation() : IamValid(true) { }
    ~TestImplementation() override { IamValid = false; }

    bool IamValid;
};

TEST_F(ValidPtrTest, basicTest) {
    TestImplementation *test = new TestImplementation();
    ASSERT_TRUE(test->IamValid);

    ValidPtr<TestImplementation> ptr(test);
    ASSERT_TRUE(ptr.valid());
    delete test;
    ASSERT_FALSE(ptr.valid());
}

TEST_F(ValidPtrTest, complexTest) {
    TestImplementation *test = new TestImplementation();
    ASSERT_TRUE(test->IamValid);

    ValidPtr<TestImplementation> ptr(test);
    {
        ValidPtr<TestImplementation> ptr2(test);
        ValidPtr<TestImplementation> ptr3(test);
        ValidPtr<TestImplementation> ptr4(test);
        ASSERT_TRUE(ptr.valid());
        ASSERT_TRUE(ptr2.valid());
        ASSERT_TRUE(ptr3.valid());
        ASSERT_TRUE(ptr4.valid());
    }
    delete test;
    ASSERT_FALSE(ptr.valid());
}
