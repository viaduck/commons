/*
 * Copyright (C) 2015-2019 The ViaDuck Project
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
#include "CertStoreTest.h"

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

TEST_F(CertStoreTest, addKeyValid) {
    // add some valid keys
    uint16_t id1 = 0, id2 = 0, id3 = 0;
    ASSERT_NO_THROW(id1 = mStore.addKey(KEY_STRING(VALID_KEY_0)));
    ASSERT_NO_THROW(id2 = mStore.addKey(KEY_STRING(VALID_KEY_1)));
    ASSERT_NO_THROW(id3 = mStore.addKey(KEY_STRING(VALID_KEY_2)));

    EXPECT_FALSE(id1 == id2 || id2 == id3 || id1 == id3) << "ID assigned twice (or more)";
}

TEST_F(CertStoreTest, addKeyInvalid) {
    ASSERT_THROW(mStore.addKey(KEY_STRING(INVALID_KEY_0)), cert_error);
    ASSERT_THROW(mStore.addKey(KEY_STRING(INVALID_KEY_1)), cert_error);
}

TEST_F(CertStoreTest, mode) {
    // add some valid keys
    uint16_t id1 = 0, id2 = 0;
    ASSERT_NO_THROW(id1 = mStore.addKey(KEY_STRING(VALID_KEY_0), CertStore::Mode::ALLOW));
    ASSERT_NO_THROW(id2 = mStore.addKey(KEY_STRING(VALID_KEY_1), CertStore::Mode::ALLOW));

    mStore.setMode(id1, CertStore::Mode::DENY);
    EXPECT_EQ(CertStore::Mode::DENY, mStore.getMode(id1));

    EXPECT_EQ(CertStore::Mode::ALLOW, mStore.getMode(id2));

    mStore.setMode(id2, CertStore::Mode::DENY);
    EXPECT_EQ(CertStore::Mode::DENY, mStore.getMode(id2));
}

TEST_F(CertStoreTest, modeInvalid) {
    // add some valid keys
    uint16_t id1 = 0, id2 = 0;
    ASSERT_NO_THROW(id1 = mStore.addKey(KEY_STRING(VALID_KEY_0), CertStore::Mode::ALLOW));
    ASSERT_NO_THROW(id2 = mStore.addKey(KEY_STRING(VALID_KEY_1), CertStore::Mode::ALLOW));

    // get mode for invalid key
    ASSERT_THROW(mStore.getMode(id2+1), cert_error);

    // set mode for invalid key
    ASSERT_THROW(mStore.setMode(id2+1, CertStore::Mode::DENY), cert_error);

    EXPECT_EQ(CertStore::Mode::ALLOW, mStore.getMode(id1));
    EXPECT_EQ(CertStore::Mode::ALLOW, mStore.getMode(id2));
}

TEST_F(CertStoreTest, removeKeyValid) {
    // add some valid keys
    uint16_t id1 = 0, id2 = 0;
    ASSERT_NO_THROW(id1 = mStore.addKey(KEY_STRING(VALID_KEY_0), CertStore::Mode::DENY));
    ASSERT_NO_THROW(id2 = mStore.addKey(KEY_STRING(VALID_KEY_1), CertStore::Mode::DENY));

    // remove key now
    mStore.removeKey(id1);
    ASSERT_THROW(mStore.getMode(id1), cert_error);
    EXPECT_EQ(CertStore::Mode::DENY, mStore.getMode(id2));
}

TEST_F(CertStoreTest, removeKeyInvalid) {
    // add some valid keys
    uint16_t id1 = 0, id2 = 0;
    ASSERT_NO_THROW(id1 = mStore.addKey(KEY_STRING(VALID_KEY_0), CertStore::Mode::DENY));
    ASSERT_NO_THROW(id2 = mStore.addKey(KEY_STRING(VALID_KEY_1), CertStore::Mode::DENY));

    // remove key now
    ASSERT_THROW(mStore.removeKey(id2+1), cert_error);
    ASSERT_EQ(CertStore::Mode::DENY, mStore.getMode(id1));
    ASSERT_EQ(CertStore::Mode::DENY, mStore.getMode(id2));
}


TEST_F(CertStoreTest, check) {
    // add some valid keys
    ASSERT_NO_THROW(mStore.addKey(KEY_STRING(VALID_KEY_0), CertStore::Mode::ALLOW));
    ASSERT_NO_THROW(mStore.addKey(KEY_STRING(VALID_KEY_1), CertStore::Mode::DENY));

    using EVP_PKEY_ref = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
    using BIO_ref = std::unique_ptr<BIO, decltype(&BIO_free)>;

    EVP_PKEY_ref key0EVP(EVP_PKEY_new(), &EVP_PKEY_free);
    {
        /* Internal OpenSSL stuff */

        // create memory bio
        BIO_ref pubKeyBIO(BIO_new(BIO_s_mem()), &BIO_free);
        ASSERT_TRUE(!!pubKeyBIO) << "Internal BIO error";

        // write PEM data into bio
        String key0 = KEY_STRING(VALID_KEY_0);
        int res = BIO_write(pubKeyBIO.get(), key0.const_data(), key0.size());
        ASSERT_FALSE(res < 0 || static_cast<uint32_t>(res) != key0.size()) << "Internal BIO error";

        // parse the PEM private key data
        CertStore::RSA_ref key0RSA(PEM_read_bio_RSA_PUBKEY(pubKeyBIO.get(), nullptr, nullptr, nullptr), &RSA_free);
        ASSERT_TRUE(!!key0RSA) << "Internal BIO error";

        EVP_PKEY_set1_RSA(key0EVP.get(), key0RSA.get());
    }

    EVP_PKEY_ref key1EVP(EVP_PKEY_new(), &EVP_PKEY_free);
    {
        // create memory bio
        BIO_ref pubKeyBIO(BIO_new(BIO_s_mem()), &BIO_free);
        ASSERT_TRUE(!!pubKeyBIO) << "Internal BIO error";

        // write PEM data into bio
        String key0 = KEY_STRING(VALID_KEY_1);
        int res = BIO_write(pubKeyBIO.get(), key0.const_data(), key0.size());
        ASSERT_FALSE(res < 0 || static_cast<uint32_t>(res) != key0.size()) << "Internal BIO error";

        // parse the PEM private key data
        CertStore::RSA_ref key0RSA(PEM_read_bio_RSA_PUBKEY(pubKeyBIO.get(), nullptr, nullptr, nullptr), &RSA_free);
        ASSERT_TRUE(!!key0RSA) << "Internal BIO error";

        EVP_PKEY_set1_RSA(key1EVP.get(), key0RSA.get());
    }

    EVP_PKEY_ref keyUnknownEVP(EVP_PKEY_new(), &EVP_PKEY_free);
    {
        // create memory bio
        BIO_ref pubKeyBIO(BIO_new(BIO_s_mem()), &BIO_free);
        ASSERT_TRUE(!!pubKeyBIO) << "Internal BIO error";

        // write PEM data into bio
        String key0 = KEY_STRING(VALID_KEY_2);
        int res = BIO_write(pubKeyBIO.get(), key0.const_data(), key0.size());
        ASSERT_FALSE(res < 0 || static_cast<uint32_t>(res) != key0.size()) << "Internal BIO error";

        // parse the PEM private key data
        CertStore::RSA_ref key0RSA(PEM_read_bio_RSA_PUBKEY(pubKeyBIO.get(), nullptr, nullptr, nullptr), &RSA_free);
        ASSERT_TRUE(!!key0RSA) << "Internal BIO error";

        EVP_PKEY_set1_RSA(keyUnknownEVP.get(), key0RSA.get());
    }

    EXPECT_EQ(1, mStore.verify(0, key0EVP.get()));
    EXPECT_EQ(0, mStore.verify(1, key1EVP.get()));
    EXPECT_EQ(0, mStore.verify(0, keyUnknownEVP.get()));
}
