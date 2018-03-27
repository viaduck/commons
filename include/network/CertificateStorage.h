#ifndef COMMONS_CERTIFICATESTORAGE_H
#define COMMONS_CERTIFICATESTORAGE_H

#include <secure_memory/Buffer.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <openssl/ssl.h>

/**
 * Storage for certificate verification management
 */
class CertificateStorage {
public:
    using RSA_ref = std::unique_ptr<RSA, decltype(&RSA_free)>;
    using BIO_ref = std::unique_ptr<BIO, decltype(&BIO_free)>;
    using EVP_PKEY_ref = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

    /**
     * Operational mode
     */
    enum class Mode {
        DENY,           /**< Deny the public key/certificate/.. **/
        ALLOW,          /**< Allow the public key/certificate/.. **/
        UNDECIDED,      /**< Undecided behavior **/
    };

    /**
     * Data structure for stored public keys
     */
    struct PublicKey {
        /**
         * Unique id in CertificateStorage
         */
        uint16_t id;
        /**
         * Operational mode for this key
         */
        Mode mode;
        /**
         * Key data as OpenSSL structure
         */
        RSA_ref data;

        /**
         * Creates an empty PublicKey with nullptr data
         */
        PublicKey() : id(0), mode(Mode::UNDECIDED), data(nullptr, &RSA_free) { }

        /**
         * Creates an PublicKey with supplied data
         * @param id Unique id
         * @param mode Operational mode
         * @param data Key data as OpenSSL structure
         */
        PublicKey(uint16_t id, Mode mode, RSA_ref &data) : id(id), mode(mode), data(std::move(data)) { }

        /**
         * Move constructor to support map insertion operations
         * @param other Other public key, data is set to nullptr
         */
        PublicKey(PublicKey &&other) : id(other.id), mode(other.mode), data(std::move(other.data)) { }
    };

    /**
     * Result codes for various operations
     */
    enum class Result {
        SUCCESS,            /**< Successful operation **/
        INVALID_FORMAT,     /**< Supplied key/certificate/.. has wrong format **/
        NOT_FOUND,          /**< Supplied key/certificate/.. was not found format **/
        INTERNAL,           /**< Internal OpenSSL failure **/
    };

    /**
     * @return Singleton application-wide instance
     */
    static CertificateStorage &getInstance() {
        return mInstance;
    }

    /**
     * @return OpenSSL data index for user supplied data to verification callback function
     */
    static int getOpenSSLDataIndex() {
        if (mOpensslDataIndex == -1)
            mOpensslDataIndex = SSL_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);
        return mOpensslDataIndex;
    }

    /**
     * Creats an empty CertificateStorage
     */
    CertificateStorage() { }

    /**
     * Adds a public key to certificate storage
     * @param key Buffer containing PEM encoded public key
     * @param mode Operational mode for this key
     * @param id Assigned ID
     * @return Any of Result::SUCCESS, Result::INVALID_FORMAT, Result::INTERNAL
     */
    Result addPublicKey(const Buffer &key, uint16_t &id, Mode mode = Mode::ALLOW);

    /**
     * Changes the operational mode of a key
     * @param id Key's ID
     * @param newMode New operational mode
     * @return Any of Result::SUCCESS, Result::NOT_FOUND, Result::INTERNAL
     */
    Result setMode(uint16_t id, Mode newMode);

    /**
     * Gets the operational mode of a key
     * @param id Key's ID
     * @param mode Operational mode
     * @return Any of Result::SUCCESS, Result::NOT_FOUND, Result::INTERNAL
     */
    Result getMode(uint16_t id, Mode &mode);

    /**
     * Removes a key from the storage
     * @param id Key's ID
     * @return Any of Result::SUCCESS, Result::NOT_FOUND, Result::INTERNAL
     */
    Result removePublicKey(uint16_t id);


    /**
     * Checks a key against the storage
     * @param key Public key to check
     * @param defaultMode If undecided (key in storage) use this Mode
     * @return Operational mode of key
     */
    Mode check(const EVP_PKEY *key, Mode defaultMode = Mode::DENY);

protected:
    static CertificateStorage mInstance;
    static int mOpensslDataIndex;

    std::unordered_map<uint16_t, PublicKey> mPublicKeys;
    uint16_t mNewPublicKeyId = 0;
    std::mutex mLockKeys;
};

#endif //COMMONS_CERTIFICATESTORAGE_H
