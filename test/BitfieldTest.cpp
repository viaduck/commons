//
// Created by steffen on 27.10.16.
//

#include <libCom/Bitfield.h>
#include "BitfieldTest.h"

TEST_F(BitfieldTest, Simple8bit) {
    {
        uint8_t a = 0;
        Bitfield::set(0, 1, 1_b, a);
        EXPECT_EQ(1_b, Bitfield::get<uint8_t>(0, 1, a));
        EXPECT_EQ(1_b, a);

        Bitfield::set(0, 1, 0_b, a);
        EXPECT_EQ(0_b, Bitfield::get<uint8_t>(0, 1, a));
        EXPECT_EQ(0_b, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(0, 4, 1001_b, a);
        EXPECT_EQ(1001_b, Bitfield::get<uint8_t>(0, 4, a));
        EXPECT_EQ(1001_b, a);

        Bitfield::set(0, 4, 0000_b, a);
        EXPECT_EQ(0_b, Bitfield::get<uint8_t>(0, 4, a));
        EXPECT_EQ(0_b, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(2, 3, 101_b, a);
        EXPECT_EQ(101_b, Bitfield::get<uint8_t>(2, 3, a));
        EXPECT_EQ(10100_b, a);

        Bitfield::set(2, 3, 000_b, a);
        EXPECT_EQ(0_b, Bitfield::get<uint8_t>(2, 3, a));
        EXPECT_EQ(0_b, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(7, 1, 1_b, a);
        EXPECT_EQ(1_b, Bitfield::get<uint8_t>(7, 1, a));
        EXPECT_EQ(10000000_b, a);

        Bitfield::set(7, 1, 0_b, a);
        EXPECT_EQ(0_b, Bitfield::get<uint8_t>(7, 1, a));
        EXPECT_EQ(0_b, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(0, 8, 10101010_b, a);
        EXPECT_EQ(10101010_b, Bitfield::get<uint8_t>(0, 8, a));
        EXPECT_EQ(10101010_b, a);

        Bitfield::set(0, 8, 00000000_b, a);
        EXPECT_EQ(0_b, Bitfield::get<uint8_t>(0, 8, a));
        EXPECT_EQ(0_b, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(4, 4, 1010_b, a);
        EXPECT_EQ(1010_b, Bitfield::get<uint8_t>(4, 4, a));
        EXPECT_EQ(10100000_b, a);

        Bitfield::set(4, 4, 0000_b, a);
        EXPECT_EQ(0_b, Bitfield::get<uint8_t>(4, 4, a));
        EXPECT_EQ(0_b, a);
    }
}

TEST_F(BitfieldTest, Complex8bit) {
    // multiple sets and unsets
    {
        uint8_t a = 0;
        Bitfield::set(4, 4, 1010_b, a);
        EXPECT_EQ(10100000_b, a);

        Bitfield::set(1, 2, 10_b, a);
        EXPECT_EQ(10100100_b, a);

        Bitfield::set(5, 3, 111_b, a);
        EXPECT_EQ(11100100_b, a);

        Bitfield::set(4, 2, 00_b, a);
        EXPECT_EQ(11000100_b, a);

        Bitfield::set(4, 4, 0010_b, a);
        EXPECT_EQ(00100100_b, a);
    }
}

TEST_F(BitfieldTest, Exceed8bit) {
    {
        uint8_t a = 0;
        // this strips one bit on the left, but leaving the rest intact
        Bitfield::set(5, 4, 1010_b, a);
        // when getting 4 bits where 1 is out of bounds, it will be set to 0
        EXPECT_EQ(0010_b, Bitfield::get<uint8_t>(5, 4, a));
        EXPECT_EQ(01000000_b, a);
    }
}

TEST_F(BitfieldTest, Complex64bit) {
    // multiple sets and unsets
    {
        uint64_t a = 0;
        Bitfield::set(20, 10, 1100101010_b, a);
        EXPECT_EQ(110010101000000000000000000000_b, a);

        Bitfield::set(3, 5, 10111_b, a);
        EXPECT_EQ(110010101000000000000010111000_b, a);

        Bitfield::set(17, 7, 0110101_b, a);
        EXPECT_EQ(110010011010100000000010111000_b, a);
    }
}
