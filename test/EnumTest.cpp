#include "test/TestEnum.h"

#include "EnumTest.h"

TEST_F(EnumTest, ToString) {
    EXPECT_STREQ("TestEnum::VALUE_1", toString(TestEnum::VALUE_1).c_str());
    EXPECT_STREQ("TestEnum::VALUE_3", toString(TestEnum::VALUE_3).c_str());
    EXPECT_STREQ("TestEnum::VALUE_BLA", toString(TestEnum::VALUE_BLA).c_str());
    EXPECT_STREQ("TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY", toString(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY).c_str());
    EXPECT_STREQ("TestEnum::INVALID_ENUM_VALUE", toString(TestEnum::INVALID_ENUM_VALUE).c_str());

    ASSERT_TRUE(true) << TestEnum::VALUE_BLA;       // compilation test for ostream
}

TEST_F(EnumTest, ToInt) {
    EXPECT_EQ(0u, toInt(TestEnum::VALUE_1));
    EXPECT_EQ(1u, toInt(TestEnum::VALUE_3));
    EXPECT_EQ(2u, toInt(TestEnum::VALUE_BLA));
    EXPECT_EQ(3u, toInt(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY));
    EXPECT_EQ(4u, toInt(TestEnum::INVALID_ENUM_VALUE));
}

TEST_F(EnumTest, FromInt) {
    EXPECT_EQ(TestEnum::VALUE_1, toTestEnum(0));
    EXPECT_EQ(TestEnum::VALUE_3, toTestEnum(1));
    EXPECT_EQ(TestEnum::VALUE_BLA, toTestEnum(2));
    EXPECT_EQ(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY, toTestEnum(3));
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, toTestEnum(4));

    // invalid enum values
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, toTestEnum(5));
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, toTestEnum(42));
}
