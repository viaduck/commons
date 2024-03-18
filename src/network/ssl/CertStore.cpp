/*
 * Copyright (C) 2015-2024 The ViaDuck Project
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

#include "../native/Native.h"
#include <network/ssl/CertStore.h>

CertStore CertStore::mInstance;

using BIO_ref = std::unique_ptr<BIO, decltype(&BIO_free)>;

uint16_t CertStore::addKey(const Buffer &key, CertStore::Mode mode) {
    // create memory bio
    BIO_ref pubKeyBIO(BIO_new(BIO_s_mem()), &BIO_free);
    L_assert(pubKeyBIO, cert_error);

    // write PEM data into bio
    int res = BIO_write(pubKeyBIO.get(), key.const_data(), key.size());
    L_assert(res >= 0 && static_cast<uint32_t>(res) == key.size(), cert_error);

    // parse the PEM public key data
    EVP_PKEY_ref pubKey(PEM_read_bio_PUBKEY(pubKeyBIO.get(), nullptr, nullptr, nullptr), &EVP_PKEY_free);
    L_assert(pubKey, cert_error);

    // operations on key container and key id need to be guarded
    std::lock_guard<std::mutex> guard(mLock);

    uint16_t id = mNextID++;
    PublicKey storedKey(mode, pubKey);
    mPublicKeys.insert(std::make_pair(id, std::move(storedKey)));
    return id;
}

void CertStore::setMode(uint16_t id, CertStore::Mode mode) {
    // operations on key container and key id need to be guarded
    std::lock_guard<std::mutex> guard(mLock);

    // find key
    auto elem = mPublicKeys.find(id);
    L_assert(elem != mPublicKeys.end(), cert_error);

    // assign new mode
    elem->second.mode = mode;
}

CertStore::Mode CertStore::getMode(uint16_t id) {
    // operations on key container and key id need to be guarded
    std::lock_guard<std::mutex> guard(mLock);

    // find key
    auto elem = mPublicKeys.find(id);
    L_assert(elem != mPublicKeys.end(), cert_error);

    // return mode
    return elem->second.mode;
}

void CertStore::removeKey(uint16_t id) {
    // operations on key container and key id need to be guarded
    std::lock_guard<std::mutex> guard(mLock);

    // find key
    auto elem = mPublicKeys.find(id);
    L_assert(elem != mPublicKeys.end(), cert_error);

    // remove key, this frees its resources as well
    mPublicKeys.erase(elem);
}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L && !defined(EVP_PKEY_cmp)
    #define EVP_PKEY_cmp EVP_PKEY_eq
#endif

bool CertStore::verify(bool pre, const EVP_PKEY *key) const {
    for (auto &publicKey : mPublicKeys) {
        // compare publicKey to the given key
        if (EVP_PKEY_cmp(key, publicKey.second.key.get()) == 1) {
            switch (publicKey.second.mode) {
                case Mode::DENY:
                    return false;
                case Mode::ALLOW:
                    return true;
                case Mode::UNDECIDED:
                    return pre;
            }
        }

        // does not match, different key types, unsupported
    }

    // default: pre
    return pre;
}
