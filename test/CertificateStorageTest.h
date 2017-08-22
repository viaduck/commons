#ifndef VDCOMMONS_CERTIFICATESTORAGETEST_H
#define VDCOMMONS_CERTIFICATESTORAGETEST_H

#include <gtest/gtest.h>
#include <commons/network/CertificateStorage.h>

class CertificateStorageTest : public ::testing::Test {
protected:
    void SetUp() override;

    CertificateStorage mStorage;
};


#endif //VDCOMMONS_CERTIFICATESTORAGETEST_H
