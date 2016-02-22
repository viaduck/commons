#include "ValidPtrTest.h"

#include <libCom/ValidPtr.h>


class TestImplementation : public ValidPtrObject<TestImplementation> {
public:
    TestImplementation() : IamValid(true) { }
    ~TestImplementation() override { IamValid = false; }

    bool IamValid;
};

TEST_F(ValidPtrTest, basicTest) {
    TestImplementation *test = new TestImplementation();
    ASSERT_TRUE(test->IamValid);

    ValidPtr<TestImplementation> ptr(test);
    ASSERT_TRUE(ptr());
    delete test;
    ASSERT_FALSE(ptr());
}

TEST_F(ValidPtrTest, complexTest) {
    TestImplementation *test = new TestImplementation();
    ASSERT_TRUE(test->IamValid);

    ValidPtr<TestImplementation> ptr(test);
    {
        ValidPtr<TestImplementation> ptr2(test);
        ValidPtr<TestImplementation> ptr3(test);
        ValidPtr<TestImplementation> ptr4(test);
        ASSERT_TRUE(ptr());
        ASSERT_TRUE(ptr2());
        ASSERT_TRUE(ptr3());
        ASSERT_TRUE(ptr4());
    }
    delete test;
    ASSERT_FALSE(ptr());
}
