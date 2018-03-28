/*
 * Copyright (C) 2015-2018 The ViaDuck Project
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

#include <openssl/pem.h>
#include <network/CertificateStorage.h>

CertificateStorage CertificateStorage::mInstance;
int CertificateStorage::mOpensslDataIndex = -1;

CertificateStorage::Result CertificateStorage::addPublicKey(const Buffer &key, uint16_t &id, CertificateStorage::Mode mode) {
    // create memory bio
    BIO_ref pubKeyStoredBio(BIO_new(BIO_s_mem()), &BIO_free);
    if (!pubKeyStoredBio)
        return Result::INTERNAL;

    // write PEM data into bio
    int res = BIO_write(pubKeyStoredBio.get(), key.const_data(), key.size());
    if (res < 0 || static_cast<uint32_t>(res) != key.size())
        return Result::INTERNAL;

    // parse the PEM private key data
    RSA_ref pubKeyStoredRSA(PEM_read_bio_RSA_PUBKEY(pubKeyStoredBio.get(), nullptr, nullptr, nullptr), &RSA_free);
    if (!pubKeyStoredRSA)
        return Result::INVALID_FORMAT;

    // operations on key container and key id need to be guarded
    {
        std::lock_guard<std::mutex> guard(mLockKeys);
        id = mNewPublicKeyId++;
        PublicKey storedKey(id, mode, pubKeyStoredRSA);
        mPublicKeys.insert(std::make_pair(id, std::move(storedKey)));
    }

    return Result::SUCCESS;
}

CertificateStorage::Result CertificateStorage::setMode(uint16_t id, CertificateStorage::Mode newMode) {
    auto elem = mPublicKeys.find(id);

    if (elem == mPublicKeys.end())
        return Result::NOT_FOUND;

    // Key's id is not the same as the map-element's key..
    if ((*elem).first != id)
        return Result::INTERNAL;        // ..there is some big fuckup happening

    // operations on key container and key id need to be guarded
    {
        std::lock_guard<std::mutex> guard(mLockKeys);
        // assign new mode
        (*elem).second.mode = newMode;
    }

    return Result::SUCCESS;
}

CertificateStorage::Result CertificateStorage::getMode(uint16_t id, CertificateStorage::Mode &mode) {
    auto elem = mPublicKeys.find(id);

    if (elem == mPublicKeys.end())
        return Result::NOT_FOUND;

    // Key's id is not the same as the map-element's key..
    if ((*elem).first != id)
        return Result::INTERNAL;        // ..there is some big fuckup happening

    mode = (*elem).second.mode;

    return Result::SUCCESS;
}

CertificateStorage::Result CertificateStorage::removePublicKey(uint16_t id) {
    auto elem = mPublicKeys.find(id);

    if (elem == mPublicKeys.end())
        return Result::NOT_FOUND;

    // Key's id is not the same as the map-element's key..
    if ((*elem).first != id)
        return Result::INTERNAL;        // ..there is some big fuckup happening

    // operations on key container and key id need to be guarded
    {
        std::lock_guard<std::mutex> guard(mLockKeys);
        // remove key, this frees its resources as well
        mPublicKeys.erase(elem);
    }

    return Result::SUCCESS;
}

CertificateStorage::Mode CertificateStorage::check(const EVP_PKEY *key, CertificateStorage::Mode defaultMode) {
    for (auto &item: mPublicKeys) {
        EVP_PKEY_ref pubKeyStored(EVP_PKEY_new(), &EVP_PKEY_free);
        EVP_PKEY_set1_RSA(pubKeyStored.get(), item.second.data.get());

        int cmpRes = EVP_PKEY_cmp(key, pubKeyStored.get());
        switch (cmpRes) {
            case 1:     // match
                return item.second.mode;
            case 0:     // do not match
            case -1:    // different key types
            case -2:    // unsupported
            default:
                continue;
        }
    }
    return defaultMode;
}
