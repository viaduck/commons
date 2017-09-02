#include "ConstexprStringTest.h"

#include <commons/ConstexprString.h>

TEST_F(ConstexprStringTest, Trivial) {
    ConstexprString<3> s("123");

    EXPECT_EQ(3u, s.size());
    EXPECT_STREQ("123", s.c_str());
}

TEST_F(ConstexprStringTest, InferArguments) {
    auto s = MakeConstexprString("123");

    EXPECT_EQ(3u, s.size());
    EXPECT_STREQ("123", s.c_str());
}

TEST_F(ConstexprStringTest, Append) {
    ConstexprString<3> s1("123");
    ConstexprString<2> s2("45");

    ConstexprString<5> s3 = s1 + s2;

    EXPECT_EQ(5u, s3.size());
    EXPECT_STREQ("12345", s3.c_str());
}
