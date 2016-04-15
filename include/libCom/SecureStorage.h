//
// Created by John Watson on 16.01.2016.
//

#ifndef CORE_SECURESTORAGE_H
#define CORE_SECURESTORAGE_H

#include <libCom/Buffer.h>
#include <openssl/rand.h>

/**
 * Creates a secure RAM storage buffer, which also prevents tampering with data in RAM
 */
template <typename K>
class SecureStorage {
public:
    /**
     * Default constructor creating an empty SecureStorage
     */
    SecureStorage() : mExtra(K::EXTRA_SIZE), mKey(32), mInternal(0) { }

    /**
     * Stores the content in provided buffer to secure RAM storage, overwriting existing content
     *
     * @param b Buffer to store
     *
     * @return True on success
     */
    template <typename T>
    bool store(const T &b) {
        mInternal.clear();

        Buffer temp;
        b.serialize(temp);

        // generate new key for each storage operation
        return reset_key() && encrypt(temp);
    }

    /**
     * Gets the content stored in secure RAM storage, content will be decrypted only for the lifetime of the callback
     *
     * @param cb Callback to receive cleartext Buffer
     *
     * @return True on success
     */
    template <typename T>
    bool get(std::function<void(const T&)> cb) const {
        Buffer tempBuf;
        T temp;

        if(decrypt(tempBuf) && temp.deserialize(tempBuf)) {
            // call cb with decrypted buffer
            cb(temp);
            return true;
        }
        return false;
    }

private:
    // encryption members
    Buffer mExtra;
    Buffer mKey;
    Buffer mInternal;

    /**
     * Encrypts b and stores it in mInternal
     *
     * @param b Buffer of data to store in mInternal
     *
     * @return True on success
     */
    bool encrypt(const Buffer &b) {
        K encryption(mKey);
        return encryption.encrypt(b, BufferRange(mInternal, 0, b.size()), mExtra);
    }

    /**
     * Decrypts mInternal and stores cleartext in b
     *
     * @param b Buffer to store cleartext in
     *
     * @return True on success
     */
    bool decrypt(Buffer &b) const {
        K decryption(mKey);
        return decryption.decrypt(mInternal, BufferRange(b, 0, mInternal.size()), mExtra);
    }

    /**
     * Generates a random key used for mInternal
     *
     * @return True on success
     */
    bool reset_key() {
        mKey.clear();

        // try to generate 512 bit key for XTS
        bool success = RAND_bytes(static_cast<uint8_t*>(mKey.data()), 256 / 8) == 1;

        // use bytes if generation succeeded
        if(success)
            mKey.use(512 /8);

        return success;
    }
};
#endif //CORE_SECURESTORAGE_H
