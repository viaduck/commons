#include <secure_memory/String.h>
#include <commons/SecureStorage.h>
#include "SecureStorageTest.h"

class DummyCrypter {
public:
    DummyCrypter (const Buffer &) {

    }

    bool encrypt(const BufferRangeConst source, BufferRange destination, BufferRange) {
        for (uint32_t i = 0; i < source.size(); ++i) {
            uint8_t data = static_cast<const uint8_t*>(source.const_data())[i];
            destination.object().write(&data, 1, destination.offset()+source.size()-i-1);
        }
        return true;
    }

    bool decrypt(const BufferRangeConst source, BufferRange destination, const BufferRangeConst) {
        return encrypt(source, destination, Buffer::DEV_NULL);
    }

    const static uint32_t EXTRA_SIZE = 0;
    const static uint32_t KEY_SIZE = 32;
};

TEST_F(SecureStorageTest, testStoreGet) {
    SecureStorage<DummyCrypter> secure;

    String testBuf1("kjsaajkdfdsfdskjfdssdf");
    String testBuf2("lksdlkjdsjklsdjlksdjklsasassasasas123aqq");

    // try to store this
    ASSERT_NO_THROW(secure.store(testBuf1));

    // check that data is equal to previously stored data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
        ASSERT_TRUE(data == testBuf1);
    }));

    // try to store something else
    ASSERT_NO_THROW(secure.store(testBuf2));

    // check that data is equal to newly stored data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
        ASSERT_TRUE(data == testBuf2);
    }));
}

TEST_F(SecureStorageTest, testModify) {
    SecureStorage<DummyCrypter> secure;

    String testBuf1("kjsaajkdfdsfdskjfdssdf");
    String testBuf2("assasasas123aqq");
    String testBufMod = testBuf1 + testBuf2;

    // try to store this
    ASSERT_NO_THROW(secure.store(testBuf1));

    // check that data is equal to previously stored data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
        ASSERT_TRUE(data == testBuf1);
    }));

    // modify test buffer
    ASSERT_TRUE(secure.modify<Buffer>([&] (Buffer &data) {
        data.append(testBuf2);
    }));

    // check that data is equal to modified data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
        ASSERT_TRUE(data == testBufMod);
    }));

    // modify test buffer same size
    *static_cast<uint8_t*>(testBufMod.data()) = 'a';
    ASSERT_TRUE(secure.modify<Buffer>([&] (Buffer &data) {
        *static_cast<uint8_t*>(data.data(0)) = 'a';
    }));

    // check that data is equal to modified data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
        ASSERT_TRUE(data == testBufMod);
    }));

    // modify test buffer smaller size
    ASSERT_TRUE(secure.modify<Buffer>([&] (Buffer &data) {
        data.clear();
    }));

    // check that data is equal to modified data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
        ASSERT_EQ(0u, data.size());
    }));
}
