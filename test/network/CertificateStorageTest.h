#ifndef COMMONS_CERTIFICATESTORAGETEST_H
#define COMMONS_CERTIFICATESTORAGETEST_H

#include <gtest/gtest.h>
#include <network/CertificateStorage.h>

class CertificateStorageTest : public ::testing::Test {
protected:
    void SetUp() override;

    CertificateStorage mStorage;
};


#endif //COMMONS_CERTIFICATESTORAGETEST_H
