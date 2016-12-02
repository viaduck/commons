#include "test/TestEnum.h"

#include "EnumTest.h"

TEST_F(EnumTest, ToString) {
    EXPECT_STREQ("TestEnum::VALUE_1", toString(TestEnum::VALUE_1).c_str());
    EXPECT_STREQ("TestEnum::VALUE_3", toString(TestEnum::VALUE_3).c_str());
    EXPECT_STREQ("TestEnum::VALUE_BLA", toString(TestEnum::VALUE_BLA).c_str());
    EXPECT_STREQ("TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY", toString(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY).c_str());
    ASSERT_TRUE(true) << TestEnum::VALUE_BLA;       // compilation test for ostream
}
