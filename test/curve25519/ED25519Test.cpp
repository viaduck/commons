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

#include <curve25519/curve25519.h>
#include "ED25519Test.h"
#include "file_test.h"

TEST_F(ED25519Test, testSignature) {
    FileTest t("ed25519_tests.txt");
    ASSERT_TRUE(t.is_open());

    std::vector<uint8_t> private_key, public_key, message, expected_signature;

    FileTest::ReadResult ret;
    for(int i = 0; (ret = t.ReadNext()) == FileTest::kReadSuccess; i++) {
        EXPECT_TRUE(t.GetBytes(&private_key, "PRIV"));
        EXPECT_EQ(static_cast<uint32_t>(ED25519_PRIVATE_KEY_LEN), private_key.size());
        EXPECT_TRUE(t.GetBytes(&public_key, "PUB"));
        EXPECT_EQ(static_cast<uint32_t>(ED25519_PUBLIC_KEY_LEN), public_key.size());
        EXPECT_TRUE(t.GetBytes(&message, "MESSAGE"));
        EXPECT_TRUE(t.GetBytes(&expected_signature, "SIG"));
        EXPECT_EQ(static_cast<uint32_t>(ED25519_SIGNATURE_LEN), expected_signature.size());

        uint8_t signature[64];
        EXPECT_EQ(1, ED25519_sign(signature, message.data(), message.size(), private_key.data())) << "failed at " << i;
        EXPECT_TRUE(t.ExpectBytesEqual(expected_signature.data(), expected_signature.size(), signature, sizeof(signature)));
        EXPECT_EQ(1, ED25519_verify(message.data(), message.size(), signature, public_key.data()));
    }

    ASSERT_EQ(FileTest::kReadEOF, ret);
}
