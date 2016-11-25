#ifndef LIBCOM_CERTIFICATESTORAGETEST_H
#define LIBCOM_CERTIFICATESTORAGETEST_H


#include <gtest/gtest.h>
#include <libCom/network/CertificateStorage.h>

class CertificateStorageTest : public ::testing::Test {
protected:
    void SetUp() override;

    CertificateStorage mStorage;
};


#endif //LIBCOM_CERTIFICATESTORAGETEST_H
