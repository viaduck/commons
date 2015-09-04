//
// Created by steffen on 21.07.15.
//

#include "AutoPtrTest.h"

TEST_F(AutoPtrTest, ScopeInt) {
    int *dataPtr = nullptr;
    long dataPtrAddress = 0;
    {
        SecureAutoPtr<int> bla(new int(123));
        long dataAddr = reinterpret_cast<long>(bla.get());
        EXPECT_EQ(123, *bla);
        *bla = 456;
        EXPECT_EQ(456, *bla);
        dataPtr = bla.get();
        EXPECT_EQ(dataAddr, reinterpret_cast<long>(bla.get()));
        dataPtrAddress = reinterpret_cast<long>(dataPtr);
        ASSERT_TRUE(dataPtr != nullptr);
        ASSERT_TRUE(dataPtrAddress != 0);
    }
    ASSERT_TRUE(dataPtr != nullptr);
    ASSERT_TRUE(dataPtrAddress != 0);
    EXPECT_EQ(dataPtrAddress, reinterpret_cast<long>(dataPtr));

    if (*dataPtr == 0) {
        SUCCEED();
        return;
    } else if (*dataPtr == 0xADADADAD) {
        SUCCEED();
        return;
    }
    FAIL();
}

TEST_F(AutoPtrTest, ScopeCharString) {
    char *dataPtr = nullptr;
    long dataPtrAddress = 0;
    {
        SecureUniquePtr<char[]> bla(5);
        long dataAddr = reinterpret_cast<long>(bla().get());
        bla()[0] = static_cast<int>('a');
        bla()[1] = static_cast<int>('b');
        bla()[2] = static_cast<int>('c');
        bla()[3] = static_cast<int>('d');
        bla()[4] = 0;

        EXPECT_STREQ("abcd", bla().get());

        bla()[0] = static_cast<int>('1');
        bla()[1] = static_cast<int>('2');
        bla()[2] = static_cast<int>('3');
        bla()[3] = static_cast<int>('4');
        bla()[4] = 0;

        EXPECT_STREQ("1234", bla().get());

        EXPECT_EQ(dataAddr, reinterpret_cast<long>(bla().get()));
        dataPtr = bla().get();
        dataPtrAddress = reinterpret_cast<long>(dataPtr);
        ASSERT_TRUE(dataPtr != nullptr);
        ASSERT_TRUE(dataPtrAddress != 0);
    }

    ASSERT_TRUE(dataPtr != nullptr);
    ASSERT_TRUE(dataPtrAddress != 0);
    EXPECT_EQ(dataPtrAddress, reinterpret_cast<long>(dataPtr));


    if (*dataPtr == 0) {
        SUCCEED();
        return;
    } else {
        ASSERT_NE('1', dataPtr[0]);
        ASSERT_NE('2', dataPtr[1]);
        ASSERT_NE('3', dataPtr[2]);
        ASSERT_NE('4', dataPtr[3]);
    }
}

TEST_F(AutoPtrTest, ScopeInt2) {
    int *dataPtr = nullptr;
    long dataPtrAddress = 0;
    const int meg = 104857600;
    const size_t megC = meg/sizeof(int);
    {
        SecureUniquePtr<int[]> bla(megC);       // 100 MB
        ASSERT_EQ(megC, bla.getSize());
        long dataAddr = reinterpret_cast<long>(bla().get());
        for (int i = 0; i < megC; ++i) {
            bla()[i] = i;
        }

        for (int i = 0; i < megC; ++i) {
            ASSERT_EQ(i, bla()[i]);
        }

        // ----

        for (int i = 0; i < megC; ++i) {
            bla()[i] = megC-i;
        }
        for (int i = 0; i < megC; ++i) {
            ASSERT_EQ(megC-i, bla()[i]);
        }

        EXPECT_EQ(dataAddr, reinterpret_cast<long>(bla().get()));
        dataPtr = bla().get();
        dataPtrAddress = reinterpret_cast<long>(dataPtr);
        ASSERT_TRUE(dataPtr != nullptr);
        ASSERT_TRUE(dataPtrAddress != 0);
    }

    ASSERT_TRUE(dataPtr != nullptr);
    ASSERT_TRUE(dataPtrAddress != 0);
    EXPECT_EQ(dataPtrAddress, reinterpret_cast<long>(dataPtr));

    for (int i = 0; i < megC; ++i) {
        ASSERT_NE(megC-i, dataPtr[i]);
    }
}