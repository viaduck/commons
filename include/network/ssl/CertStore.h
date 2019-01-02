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

#ifndef COMMONS_CERTIFICATESTORAGE_H
#define COMMONS_CERTIFICATESTORAGE_H

#include <atomic>
#include <mutex>
#include <unordered_map>

#include <openssl/rsa.h>

#include <commons/util/Except.h>
#include <secure_memory/Buffer.h>

DEFINE_ERROR(cert, base_error);

/**
 * Storage for certificate verification management
 */
class CertStore {
public:
    using RSA_ref = std::unique_ptr<RSA, decltype(&RSA_free)>;

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
        PublicKey() : mode(Mode::UNDECIDED), data(nullptr, &RSA_free) { }

        /**
         * Creates an PublicKey with supplied data
         * @param mode Operational mode
         * @param data Key data as OpenSSL structure
         */
        PublicKey(Mode mode, RSA_ref &data) : mode(mode), data(std::move(data)) { }

        /**
         * Move constructor to support map insertion operations
         * @param other Other public key, data is set to nullptr
         */
        PublicKey(PublicKey &&other) noexcept : mode(other.mode), data(std::move(other.data)) { }
    };

    /**
     * @return Singleton application-wide instance
     */
    static CertStore &getInstance() {
        return mInstance;
    }

    /**
     * Creates an empty CertificateStorage
     */
    explicit CertStore() = default;

    /**
     * Adds a public key to certificate storage
     *
     * @param key Buffer containing PEM encoded public key
     * @param mode Operational mode for this key
     * @return Assigned ID for the key
     */
    uint16_t addKey(const Buffer &key, Mode mode = Mode::ALLOW);

    /**
     * Changes the operational mode of a key
     *
     * @param id Key's ID
     * @param mode New operational mode
     */
    void setMode(uint16_t id, Mode mode);

    /**
     * Gets the operational mode of a key
     *
     * @param id Key's ID
     * @return Key's operational mode
     */
    Mode getMode(uint16_t id);

    /**
     * Removes a public key from the storage
     *
     * @param id Key's ID
     */
    void removeKey(uint16_t id);

    /**
     * Checks a key against the storage
     *
     * @param pre Pre verification
     * @param key Public key to check
     * @return True if verification succeeded
     */
    bool verify(bool pre, const EVP_PKEY *key) const;

protected:
    static CertStore mInstance;

    // lock for the public key map
    std::mutex mLock;
    // public key map
    std::unordered_map<uint16_t, PublicKey> mPublicKeys;
    // next id
    uint16_t mNextID = 1;
};

#endif //COMMONS_CERTIFICATESTORAGE_H
