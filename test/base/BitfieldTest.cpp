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

#include <commons/Bitfield.h>
#include "BitfieldTest.h"

TEST_F(BitfieldTest, Simple8bit) {
    {
        uint8_t a = 0;
        Bitfield::set(0, 1, 0b1, a);
        EXPECT_EQ(0b1, Bitfield::get<uint8_t>(0, 1, a));
        EXPECT_EQ(0b1, a);

        Bitfield::set(0, 1, 0b0, a);
        EXPECT_EQ(0b0, Bitfield::get<uint8_t>(0, 1, a));
        EXPECT_EQ(0b0, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(0, 4, 0b1001, a);
        EXPECT_EQ(0b1001, Bitfield::get<uint8_t>(0, 4, a));
        EXPECT_EQ(0b1001, a);

        Bitfield::set(0, 4, 0b0000, a);
        EXPECT_EQ(0b0, Bitfield::get<uint8_t>(0, 4, a));
        EXPECT_EQ(0b0, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(2, 3, 0b101, a);
        EXPECT_EQ(0b101, Bitfield::get<uint8_t>(2, 3, a));
        EXPECT_EQ(0b10100, a);

        Bitfield::set(2, 3, 0b000, a);
        EXPECT_EQ(0b0, Bitfield::get<uint8_t>(2, 3, a));
        EXPECT_EQ(0b0, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(7, 1, 0b1, a);
        EXPECT_EQ(0b1, Bitfield::get<uint8_t>(7, 1, a));
        EXPECT_EQ(0b10000000, a);

        Bitfield::set(7, 1, 0b0, a);
        EXPECT_EQ(0b0, Bitfield::get<uint8_t>(7, 1, a));
        EXPECT_EQ(0b0, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(0, 8, 0b10101010, a);
        EXPECT_EQ(0b10101010, Bitfield::get<uint8_t>(0, 8, a));
        EXPECT_EQ(0b10101010, a);

        Bitfield::set(0, 8, 0b00000000, a);
        EXPECT_EQ(0b0, Bitfield::get<uint8_t>(0, 8, a));
        EXPECT_EQ(0b0, a);
    }

    {
        uint8_t a = 0;
        Bitfield::set(4, 4, 0b1010, a);
        EXPECT_EQ(0b1010, Bitfield::get<uint8_t>(4, 4, a));
        EXPECT_EQ(0b10100000, a);

        Bitfield::set(4, 4, 0b0000, a);
        EXPECT_EQ(0b0, Bitfield::get<uint8_t>(4, 4, a));
        EXPECT_EQ(0b0, a);
    }
}

TEST_F(BitfieldTest, Complex8bit) {
    // multiple sets and unsets
    {
        uint8_t a = 0;
        Bitfield::set(4, 4, 0b1010, a);
        EXPECT_EQ(0b10100000, a);

        Bitfield::set(1, 2, 0b10, a);
        EXPECT_EQ(0b10100100, a);

        Bitfield::set(5, 3, 0b111, a);
        EXPECT_EQ(0b11100100, a);

        Bitfield::set(4, 2, 0b00, a);
        EXPECT_EQ(0b11000100, a);

        Bitfield::set(4, 4, 0b0010, a);
        EXPECT_EQ(0b00100100, a);
    }
}

TEST_F(BitfieldTest, Exceed8bit) {
    {
        uint8_t a = 0;
        // this strips one bit on the left, but leaving the rest intact
        Bitfield::set(5, 4, 0b1010, a);
        // when getting 4 bits where 1 is out of bounds, it will be set to 0
        EXPECT_EQ(0b0010, Bitfield::get<uint8_t>(5, 4, a));
        EXPECT_EQ(0b01000000, a);
    }
}

TEST_F(BitfieldTest, Complex64bit) {
    // multiple sets and unsets
    {
        uint64_t a = 0;
        Bitfield::set(20, 10, 0b1100101010, a);
        EXPECT_EQ(0b110010101000000000000000000000u, a);

        Bitfield::set(3, 5, 0b10111, a);
        EXPECT_EQ(0b110010101000000000000010111000u, a);

        Bitfield::set(17, 7, 0b0110101, a);
        EXPECT_EQ(0b110010011010100000000010111000u, a);
    }
}

TEST_F(BitfieldTest, InconsistentParameters) {
    {
        uint8_t a = 0;
        // We say, we want to set only 4 bits, but pass 6 bits. Clamping must be done.
        Bitfield::set(0, 4, 0b111111, a);
        EXPECT_EQ(0b1111, a);
    }
}
