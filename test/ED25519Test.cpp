//
// Created by John Watson on 27.10.16.
//

#include "curve25519.h"
#include "ED25519Test.h"
#include "file_test.h"

TEST_F(ED25519Test, testSignature) {
    FileTest t(CMAKE_CURRENT_BINARY_DIR "/ed25519_tests.txt");
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
