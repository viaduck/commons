#include <libCom/String.h>
#include <libCom/SecureStorage.h>
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
};

TEST_F(SecureStorageTest, testSecureStorage) {
    SecureStorage<DummyCrypter> secure;

    String testBuf1("kjsaajkdfdsfdskjfdssdf");
    String testBuf2("lksdlkjdsjklsdjlksdjklsasassasasas123aqq");

    // try to store this
    ASSERT_TRUE(secure.store(*testBuf1));

    // check that data is equal to previously stored data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
    ASSERT_TRUE(data == *testBuf1);
    }));

    // try to store something else
    ASSERT_TRUE(secure.store(*testBuf2));

    // check that data is equal to newly stored data
    ASSERT_TRUE(secure.get<Buffer>([&] (const Buffer& data) {
    ASSERT_TRUE(data == *testBuf2);
    }));
}
