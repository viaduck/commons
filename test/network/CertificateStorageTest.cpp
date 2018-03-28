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

#include <secure_memory/String.h>
#include <openssl/pem.h>
#include "CertificateStorageTest.h"

#define KEY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define KEY_STRING(a) String(a, KEY_SIZE(a))

const char VALID_KEY_0[] = "-----BEGIN PUBLIC KEY-----\n"
        "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA4tmmlX6LxHFfkUr+L3Tz\n"
        "Mfyw2RrkPvIgtSgtwHEIIQq5By3zsT0m8pNfpspascIQjtJ47A+HkbAgzn0tQvuI\n"
        "D9sQPbrdtHrHll5zH4jOPPuibx0dczmmXN3cBnMZZZaUMmYclwvSZ8zu3nJC8iG5\n"
        "t1ITRlnCvnNzjqHF2v2vGvfth7KcmVrb8q4wlI9kfdiuL0ypm9A/OWA0wjgQOAUq\n"
        "RXe7z1aqDU7fqyM72Vynkw7aWzg/gitWA1t7NT6Ph8aVRcTAffRBdcOA+B6kTVKc\n"
        "DonHvD0qk758ieFEED8bWdEsP+gwAVdbWD4pRfZzYYJmqCpsfFfDZ7b5DKRs0Cbr\n"
        "GRdIwZC/yXzLKKlG1MrkeXbmEfK05SyjF6R8swULDK29oms6LjxSSZPdJEB+kUf6\n"
        "aWKT0+hoRdvXJh+dMjj+WBTAoqSu5SP7yfxZePDO41MtP42MQaELFy0e0QqV2z8r\n"
        "WlMAbYweVPMtsvTplB1dCj2YB9ud1G1l2H0xtb+iapeF4gKm9hXs+JPPx++9WVGK\n"
        "gttgruOsj3ukjtVlB2gKUV5jj3B9EwQsuiGNJIsC/v9nKom3fTiLRZYkCYCCfZ/H\n"
        "tVOlJjPbHELFA5sFhVaPlX3BDQCL+w5hYVZPsZ64JZI7bkIDIS0Q8R9zsAoK+98A\n"
        "CZTMil/tviZShNIisKNfPpUCAwEAAQ==\n"
        "-----END PUBLIC KEY-----";
const char VALID_KEY_1[] = "-----BEGIN PUBLIC KEY-----\n"
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsR1orokr/no/Mja5pSbD\n"
        "AY3yCX2oz3m2nwXSLAVvu+4nRvgswRLqZgI3/qNNFVMdCrVzPfzve+WZFAUX28hP\n"
        "ZcnQGh7RtPy+QFCEqxW0x1zokxgDYwD23Z3h7v1ntiN2qBzT+nAB9anWVri+lE+f\n"
        "/6LIfzSp6fHeevqqjqeabdptNx36/uPXjVK4EFnK/mJZxOay1AS6uQ6f+tI2z6Tt\n"
        "RK2hXegEcXmQwlEbG5OhXEeyFW5KL+GRUsDQLjZo6JhcVBztpKA7UaOO8IenaFmZ\n"
        "daryoD7XYNplltaJo/UHz3D5qbjv1ei8tbkMv/u25aiVKNzxm1rdI4qFTpfocuq1\n"
        "CwIDAQAB\n"
        "-----END PUBLIC KEY-----";
const char VALID_KEY_2[] = "-----BEGIN PUBLIC KEY-----\n"
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2xUBhl4A5xUQfiFUUlTw\n"
        "89H/urcYrg8wB3wc49gA+PNBJrQ/Xwbg1ze4hUpyUqLvoHcABbiM+gKPUGmZmEtv\n"
        "8sDPsKmgVggxl+LL+RJbYabSMGsfLNuNpZfhMEH1rHxYsfezRfbHSVphp9k7Sa+9\n"
        "uCXPDoLfLbsLvmpg0sxC3DLoU6rGd39vK8iSx4VWvsr4CiV1HUE9SEaDna/RgyUt\n"
        "WPbW3TUVF1lvelE69pjkNjuiDP2+98iCfJzUBbDdmv2JpQxOBMGzEC9NBcOlLqSm\n"
        "j3DJiTVbWYzNxXLBnKxqYq1vi4c9oCXEPSdvC7OxrEhN7BmNnfhx/rntaGaGsp71\n"
        "jQIDAQAB\n"
        "-----END PUBLIC KEY-----";

const char INVALID_KEY_0[] = "-----BEGIN PUBLIC KEY-----\n"
        "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA4tmmlX6LxHFfkUr+L3Tz\n"
        "Mfyw2RrkPvIgtSgtwHEIIQq5By3zsT0m8pNfpspascIQjtJ47A+HkbAgzn0tQvuI\n"
        "D9sQPbrdtHrHll5zH4jOPPuibx0dczmmXN3cBnMZZZaUMmYclwvSZ8zu3nJC8iG5\n"
        "t1ITRlnCvnNzjqHF2v2vGvfth7KcmV4wlI9kfdiuL0ypm9A/OWA0wjgQOAUq\n"
        "RXe7z1aqDU7fqyM72Vynkw7aWzg/gitWA1t7NT6Ph8aVRcTAffRBdcOA+B6kTVKc\n"
        "DonHvD0qk758ieFEED8bWdEsP+gwAVdbWD4pRfZzYYJmqCpsfFfDZ7b5DKRs0Cbr\n"
        "GRdIwZC/yXzLKKlG1MrkeXbmEfK05SyjF6R8swULDK29oms6LjxSSZPdJEB+kUf6\n"
        "aWKT0+hoRdvXJh+dMjj+WBTAoqSu5SP7yfxZePDO41MtP42MQaELFy0e0QqV2z8r\n"
        "WlMAbYweVPMtsvTplB1dCj2YB9ud1G1l2H0xtb+iapeF4gKm9hXs+JPPx++9WVGK\n"
        "gttgruOsj3ukjtVlB2gKUV5jj3B9EwQsuiGNJIsC/v9nKom3fTiLRZYkCYCCfZ/H\n"
        "tVOlJjPbHELFA5sFhVaPlX3BDQCL+w5hYVZPsZ64JZI7bkIDIS0Q8R9zsAoK+98A\n"
        "CZTMil/tviZShNIisKNfPpUCAwEAAQ==\n"
        "-----END PUBLIC KEY-----";
const char INVALID_KEY_1[] = "-----BEGIN PUBLIC KEX-----\n"
        "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA4tmmlX6LxHFfkUr+L3Tz\n"
        "Mfyw2RrkPvIgtSgtwHEIIQq5By3zsT0m8pNfpspascIQjtJ47A+HkbAgzn0tQvuI\n"
        "D9sQPbrdtHrHll5zH4jOPPuibx0dczmmXN3cBnMZZZaUMmYclwvSZ8zu3nJC8iG5\n"
        "t1ITRlnCvnNzjqHF2v2vGvfth7KcmVrb8q4wlI9kfdiuL0ypm9A/OWA0wjgQOAUq\n"
        "RXe7z1aqDU7fqyM72Vynkw7aWzg/gitWA1t7NT6Ph8aVRcTAffRBdcOA+B6kTVKc\n"
        "DonHvD0qk758ieFEED8bWdEsP+gwAVdbWD4pRfZzYYJmqCpsfFfDZ7b5DKRs0Cbr\n"
        "GRdIwZC/yXzLKKlG1MrkeXbmEfK05SyjF6R8swULDK29oms6LjxSSZPdJEB+kUf6\n"
        "aWKT0+hoRdvXJh+dMjj+WBTAoqSu5SP7yfxZePDO41MtP42MQaELFy0e0QqV2z8r\n"
        "WlMAbYweVPMtsvTplB1dCj2YB9ud1G1l2H0xtb+iapeF4gKm9hXs+JPPx++9WVGK\n"
        "gttgruOsj3ukjtVlB2gKUV5jj3B9EwQsuiGNJIsC/v9nKom3fTiLRZYkCYCCfZ/H\n"
        "tVOlJjPbHELFA5sFhVaPlX3BDQCL+w5hYVZPsZ64JZI7bkIDIS0Q8R9zsAoK+98A\n"
        "CZTMil/tviZShNIisKNfPpUCAwEAAQ==\n"
        "-----END PUBLIC KEY-----";

void CertificateStorageTest::SetUp() {
    ::testing::Test::SetUp();
}

TEST_F(CertificateStorageTest, addKeyValid) {
    uint16_t id = 42;
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.addPublicKey(KEY_STRING(VALID_KEY_0), id));
    EXPECT_NE(42, id);       // id must be assigned

    uint16_t id2 = 42;
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.addPublicKey(KEY_STRING(VALID_KEY_1), id2));
    EXPECT_NE(42, id2);       // id must be assigned

    uint16_t id3 = 42;
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.addPublicKey(KEY_STRING(VALID_KEY_2), id3));
    EXPECT_NE(42, id3);       // id must be assigned

    EXPECT_FALSE(id == id2 || id2 == id3 || id == id3) << "ID assigned twice (or more)";
}

TEST_F(CertificateStorageTest, addKeyInvalid) {
    uint16_t id = 44423;
    EXPECT_EQ(CertificateStorage::Result::INVALID_FORMAT, mStorage.addPublicKey(KEY_STRING(INVALID_KEY_0), id));
    EXPECT_EQ(44423, id);       // id must not change!

    id = 44423;
    EXPECT_EQ(CertificateStorage::Result::INVALID_FORMAT, mStorage.addPublicKey(KEY_STRING(INVALID_KEY_1), id));
    EXPECT_EQ(44423, id);       // id must not change!
}

TEST_F(CertificateStorageTest, mode) {
    uint16_t id0 = 42;
    CertificateStorage::Mode mode0 = CertificateStorage::Mode::ALLOW;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_0), id0, mode0);
    uint16_t id1 = 42;
    CertificateStorage::Mode mode1 = CertificateStorage::Mode::ALLOW;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_1), id1, mode1);

    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.setMode(id0, CertificateStorage::Mode::DENY));
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id0, mode0));
    EXPECT_EQ(CertificateStorage::Mode::DENY, mode0);

    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id1, mode1));
    EXPECT_EQ(CertificateStorage::Mode::ALLOW, mode1) << "Other must not be tainted";

    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.setMode(id1, CertificateStorage::Mode::DENY));
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id1, mode1));
    EXPECT_EQ(CertificateStorage::Mode::DENY, mode1);
}

TEST_F(CertificateStorageTest, modeInvalid) {
    uint16_t id0 = 42;
    CertificateStorage::Mode mode0 = CertificateStorage::Mode::ALLOW;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_0), id0, mode0);
    uint16_t id1 = 42;
    CertificateStorage::Mode mode1 = CertificateStorage::Mode::ALLOW;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_1), id1, mode1);

    // get mode for invalid key
    CertificateStorage::Mode mode = CertificateStorage::Mode::ALLOW;
    EXPECT_EQ(CertificateStorage::Result::NOT_FOUND, mStorage.getMode(id1+1, mode));

    // set mode for invalid key
    EXPECT_EQ(CertificateStorage::Result::NOT_FOUND, mStorage.setMode(id1+1, CertificateStorage::Mode::DENY));

    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id0, mode0));
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id1, mode1));
    EXPECT_EQ(CertificateStorage::Mode::ALLOW, mode0) << "Other must not be tainted";
    EXPECT_EQ(CertificateStorage::Mode::ALLOW, mode1) << "Other must not be tainted";
}

TEST_F(CertificateStorageTest, removeKeyValid) {
    CertificateStorage::Mode mode = CertificateStorage::Mode::DENY;
    uint16_t id = 42;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_0), id, mode);
    uint16_t id2 = 42;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_1), id2, mode);

    // remove key now
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.removePublicKey(id));
    EXPECT_EQ(CertificateStorage::Result::NOT_FOUND, mStorage.getMode(id, mode)) << "Key must have been removed already";
    mode = CertificateStorage::Mode::ALLOW;     // reset value so that tested method really sets the value
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id2, mode)) << "Other key must still be present";
    EXPECT_EQ(CertificateStorage::Mode::DENY, mode) << "Other must not be tainted";
}

TEST_F(CertificateStorageTest, removeKeyInvalid) {
    CertificateStorage::Mode mode = CertificateStorage::Mode::DENY;
    uint16_t id = 42;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_0), id, mode);
    uint16_t id2 = 42;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_1), id2, mode);

    // remove key now
    EXPECT_EQ(CertificateStorage::Result::NOT_FOUND, mStorage.removePublicKey(id2+1));
    mode = CertificateStorage::Mode::ALLOW;     // reset value so that tested method really sets the value
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id, mode)) << "Key 1 must still be present";
    EXPECT_EQ(CertificateStorage::Mode::DENY, mode) << "Key 1 must not be tainted";
    mode = CertificateStorage::Mode::ALLOW;     // reset value so that tested method really sets the value
    EXPECT_EQ(CertificateStorage::Result::SUCCESS, mStorage.getMode(id2, mode)) << "Key 2 must still be present";
    EXPECT_EQ(CertificateStorage::Mode::DENY, mode) << "Key 2 must not be tainted";
}


TEST_F(CertificateStorageTest, check) {
    uint16_t id = 42;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_0), id, CertificateStorage::Mode::ALLOW);
    uint16_t id2 = 42;
    mStorage.addPublicKey(KEY_STRING(VALID_KEY_1), id2, CertificateStorage::Mode::DENY);

    CertificateStorage::EVP_PKEY_ref key0EVP(EVP_PKEY_new(), &EVP_PKEY_free);
    {
        /** Internal OpenSSL stuff **/
        // create memory bio
        CertificateStorage::BIO_ref pubKeyStoredBio(BIO_new(BIO_s_mem()), &BIO_free);
        ASSERT_TRUE(!!pubKeyStoredBio) << "Internal BIO error";

        // write PEM data into bio
        String key0 = KEY_STRING(VALID_KEY_0);
        int res = BIO_write(pubKeyStoredBio.get(), key0.const_data(), key0.size());
        ASSERT_FALSE(res < 0 || static_cast<uint32_t>(res) != key0.size()) << "Internal BIO error";

        // parse the PEM private key data
        CertificateStorage::RSA_ref key0RSA(PEM_read_bio_RSA_PUBKEY(pubKeyStoredBio.get(), nullptr, nullptr, nullptr),
                                            &RSA_free);
        ASSERT_TRUE(!!key0RSA) << "Internal BIO error";

        EVP_PKEY_set1_RSA(key0EVP.get(), key0RSA.get());
        /** ============= **/
    }

    CertificateStorage::EVP_PKEY_ref key1EVP(EVP_PKEY_new(), &EVP_PKEY_free);
    {
        /** Internal OpenSSL stuff **/
        // create memory bio
        CertificateStorage::BIO_ref pubKeyStoredBio(BIO_new(BIO_s_mem()), &BIO_free);
        ASSERT_TRUE(!!pubKeyStoredBio) << "Internal BIO error";

        // write PEM data into bio
        String key0 = KEY_STRING(VALID_KEY_1);
        int res = BIO_write(pubKeyStoredBio.get(), key0.const_data(), key0.size());
        ASSERT_FALSE(res < 0 || static_cast<uint32_t>(res) != key0.size()) << "Internal BIO error";

        // parse the PEM private key data
        CertificateStorage::RSA_ref key0RSA(PEM_read_bio_RSA_PUBKEY(pubKeyStoredBio.get(), nullptr, nullptr, nullptr),
                                            &RSA_free);
        ASSERT_TRUE(!!key0RSA) << "Internal BIO error";

        EVP_PKEY_set1_RSA(key1EVP.get(), key0RSA.get());
        /** ============= **/
    }

    CertificateStorage::EVP_PKEY_ref keyInvalidEVP(EVP_PKEY_new(), &EVP_PKEY_free);
    {
        /** Internal OpenSSL stuff **/
        // create memory bio
        CertificateStorage::BIO_ref pubKeyStoredBio(BIO_new(BIO_s_mem()), &BIO_free);
        ASSERT_TRUE(!!pubKeyStoredBio) << "Internal BIO error";

        // write PEM data into bio
        String key0 = KEY_STRING(VALID_KEY_2);
        int res = BIO_write(pubKeyStoredBio.get(), key0.const_data(), key0.size());
        ASSERT_FALSE(res < 0 || static_cast<uint32_t>(res) != key0.size()) << "Internal BIO error";

        // parse the PEM private key data
        CertificateStorage::RSA_ref key0RSA(PEM_read_bio_RSA_PUBKEY(pubKeyStoredBio.get(), nullptr, nullptr, nullptr),
                                            &RSA_free);
        ASSERT_TRUE(!!key0RSA) << "Internal BIO error";

        EVP_PKEY_set1_RSA(keyInvalidEVP.get(), key0RSA.get());
        /** ============= **/
    }

    EXPECT_EQ(CertificateStorage::Mode::ALLOW, mStorage.check(key0EVP.get(), CertificateStorage::Mode::DENY)) << "Key 0 must be allowed";
    EXPECT_EQ(CertificateStorage::Mode::DENY, mStorage.check(key1EVP.get(), CertificateStorage::Mode::DENY)) << "Key 1 must be denied";
    EXPECT_EQ(CertificateStorage::Mode::DENY, mStorage.check(keyInvalidEVP.get(), CertificateStorage::Mode::DENY)) << "Key 2 is not present, default policy must be used";
    EXPECT_EQ(CertificateStorage::Mode::ALLOW, mStorage.check(keyInvalidEVP.get(), CertificateStorage::Mode::ALLOW)) << "Key 2 is not present, default policy must be used";
}
