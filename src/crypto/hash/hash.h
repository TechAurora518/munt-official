// Copyright (c) 2015-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CRYPTO_HASH_HASH_H
#define CRYPTO_HASH_HASH_H

#include <hash.h>
#include <uint256.h>
#include "crypto/hash/scrypt.h"
#include "crypto/hash/city.h"

inline void hash_sha256(const void* pData, unsigned int size, unsigned char* hash)
{
    CSHA256 sha;
    sha.Write((unsigned char*)pData, size);
    sha.Finalize((unsigned char*)hash);
}

inline void hash_sha256(const void* pData, unsigned int size, arith_uint256& thash)
{
    CSHA256 sha;
    sha.Write((unsigned char*)pData, size);
    sha.Finalize((unsigned char*)&thash);
}

inline void hash_city(const void* pData, arith_uint256& thash)
{
    //For testing purposes
    uint128 temphash =  CityHash128((char*)pData, 80);
    thash |= temphash.first;
    thash <<= 64;
    thash |= temphash.second;
    thash <<= 64;
    temphash = CityHash128((char*)ArithToUint256(thash).begin(), 32);
    thash |= temphash.first;
    thash <<= 64;
    thash |= temphash.second;
}


#endif
