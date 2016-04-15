#include "ConversionTest.h"

#include <libCom/conversions.h>

TEST_F(ConversionTest, Bswap) {
    EXPECT_EQ(0x12u, bswap(static_cast<uint8_t>(0x12)));
    EXPECT_EQ(0x3412u, bswap(static_cast<uint16_t>(0x1234)));
    EXPECT_EQ(0x78563412u, bswap(static_cast<uint32_t>(0x12345678)));
    EXPECT_EQ(0x5634129078563412u, bswap(static_cast<uint64_t>(0x1234567890123456)));
}

TEST_F(ConversionTest, BswapFloat) {
    EXPECT_NE(1.23f, bswap(1.23f));
    EXPECT_EQ(1.23f, bswap(bswap(1.23f)));
}

TEST_F(ConversionTest, BswapDouble) {
    EXPECT_NE(3.14159, bswap(3.14159));
    EXPECT_EQ(3.14159, bswap(bswap(3.14159)));
}
