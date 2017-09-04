#ifndef VDCOMMONS_SECURESTORAGE_H
#define VDCOMMONS_SECURESTORAGE_H

#include <secure_memory/Buffer.h>
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
    SecureStorage() : mExtra(K::EXTRA_SIZE), mKey(K::KEY_SIZE), mInternal(0) { }

    /**
     * Stores the content in provided buffer to secure RAM storage, overwriting existing content
     *
     * @param b Buffer to store
     *
     * @return True on success
     */
    template <typename T>
    void store(const T &b) {
        mInternal.clear();

        Buffer temp;
        b.serialize(temp);

        // generate new key for each storage operation
        reset_key();
        encrypt(temp);
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

        decrypt(tempBuf);
        if (temp.deserialize(tempBuf)) {
            // call cb with decrypted buffer
            cb(temp);
            return true;
        }
        return false;
    }

    /**
     * Allows modification of the content stored in secure RAM storage, content will be decrypted only for the
     * lifetime of the callback and encrypts and stores the modified content with a new key again.
     *
     * @param cb Callback to receive cleartext Buffer
     *
     * @return True on success
     */
    template <typename T>
    bool modify(std::function<void(T&)> cb) {
        Buffer tempBuf;
        T temp;

        decrypt(tempBuf);
        if (temp.deserialize(tempBuf)) {
            // call cb with decrypted buffer
            cb(temp);

            //
            store(temp);
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
    void encrypt(const Buffer &b) {
        K encryption(mKey);
        encryption.encrypt(b, BufferRange(mInternal, 0, b.size()), mExtra);
    }

    /**
     * Decrypts mInternal and stores cleartext in b
     *
     * @param b Buffer to store cleartext in
     *
     * @return True on success
     */
    void decrypt(Buffer &b) const {
        K decryption(mKey);
        decryption.decrypt(mInternal, BufferRange(b, 0, mInternal.size()), mExtra);
    }

    /**
     * Generates a random key used for mInternal
     *
     * @return True on success
     */
    void reset_key() {
        // use buffer in advance if needed
        if (mKey.size() == 0)
            mKey.use(K::KEY_SIZE);

        // try to generate keysize byte key for encryption algo
        bool success = RAND_bytes(static_cast<uint8_t*>(mKey.data()), K::KEY_SIZE) == 1;

        // clear key on fail to make sure it always fails when used with K's methods
        if(!success)
            mKey.clear();

        // FIXME: throw here
        //return success;
    }
};
#endif //VDCOMMONS_SECURESTORAGE_H
