#include "ConversionTest.h"

#include <commons/conversions.h>

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

TEST_F(ConversionTest, BswapComplex) {
    struct ComplexStruct {
        float a;
        int b;
        double c;

        bool operator==(const ComplexStruct &other) const {
            return a == other.a && b == other.b && c == other.c;
        }

        bool operator!=(const ComplexStruct &other) const {
            return !operator==(other);
        }
    };
    float finf = std::numeric_limits<float>::infinity();
    double dinf = std::numeric_limits<double>::infinity();

    ComplexStruct a = {finf, 0, dinf};
    ComplexStruct b = {-finf, 5, -dinf};
    ComplexStruct c = {1.234f, 5, 3.14159};

    EXPECT_NE(a, bswap(a));
    EXPECT_EQ(a, bswap(bswap(a)));
    EXPECT_NE(b, bswap(b));
    EXPECT_EQ(b, bswap(bswap(b)));
    EXPECT_NE(c, bswap(c));
    EXPECT_EQ(c, bswap(bswap(c)));
}
