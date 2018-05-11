// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_KEY_H
#define GULDEN_KEY_H

#include "pubkey.h"
#include "serialize.h"
#include "support/allocators/secure.h"
#include "uint256.h"

#include <stdexcept>
#include <vector>


/**
 * secp256k1:
 * const unsigned int PRIVATE_KEY_SIZE = 279;
 * const unsigned int PUBLIC_KEY_SIZE  = 65;
 * const unsigned int SIGNATURE_SIZE   = 72;
 *
 * see www.keylength.com
 * script supports up to 75 for single byte push
 */

/**
 * secure_allocator is defined in allocators.h
 * CPrivKey is a serialized private key, with all parameters included (279 bytes)
 */
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;

/** An encapsulated private key. */
class CKey
{
private:
    //! Whether this private key is valid. We check for correctness when modifying the key
    //! data, so fValid should always correspond to the actual state.
    bool fValid;

    //! Whether the public key corresponding to this private key is (to be) compressed.
    bool fCompressed;

    //! The actual byte data
    std::vector<unsigned char, secure_allocator<unsigned char> > keydata;

    //! Check whether the 32-byte array pointed to by vch is valid keydata.
    bool static Check(const unsigned char* vch);

public:
    //! Construct an invalid private key.
    CKey() : fValid(false), fCompressed(false)
    {
        // Important: vch must be 32 bytes in length to not break serialization
        keydata.resize(32);
    }

    //! Destructor (again necessary because of memlocking).
    ~CKey()
    {
    }

    friend bool operator==(const CKey& a, const CKey& b)
    {
        return a.fCompressed == b.fCompressed &&
            a.size() == b.size() &&
            memcmp(a.keydata.data(), b.keydata.data(), a.size()) == 0;
    }

    //! Initialize using begin and end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend, bool fCompressedIn)
    {
        if (size_t(pend - pbegin) != keydata.size()) {
            fValid = false;
        } else if (Check(&pbegin[0])) {
            memcpy(keydata.data(), (unsigned char*)&pbegin[0], keydata.size());
            fValid = true;
            fCompressed = fCompressedIn;
        } else {
            fValid = false;
        }
    }

    //! Simple read-only vector-like interface.
    unsigned int size() const { return (fValid ? keydata.size() : 0); }
    const unsigned char* begin() const { return keydata.data(); }
    const unsigned char* end() const { return keydata.data() + size(); }

    //! Check whether this private key is valid.
    bool IsValid() const { return fValid; }

    //! Check whether the public key corresponding to this private key is (to be) compressed.
    bool IsCompressed() const { return fCompressed; }

    //! Generate a new private key using a cryptographic PRNG.
    void MakeNewKey(bool fCompressed);

    /**
     * Convert the private key to a CPrivKey (serialized OpenSSL private key data).
     * This is expensive.
     */
    CPrivKey GetPrivKey() const;

    /**
     * Compute the public key from a private key.
     * This is expensive.
     */
    CPubKey GetPubKey() const;

    /**
     * Create a DER-serialized signature.
     * The test_case parameter tweaks the deterministic nonce.
     */
    bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, uint32_t test_case = 0) const;

    /**
     * Create a compact signature (65 bytes), which allows reconstructing the used public key.
     * The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
     * The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
     *                  0x1D = second key with even y, 0x1E = second key with odd y,
     *                  add 0x04 for compressed keys.
     */
    bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;

    //! Derive BIP32 child key.
    bool Derive(CKey& keyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;

    /**
     * Verify thoroughly whether a private key and a public key match.
     * This is done using a different mechanism than just regenerating it.
     */
    bool VerifyPubKey(const CPubKey& vchPubKey) const;

    //! Load private key and check that public key matches.
    bool Load(CPrivKey& privkey, CPubKey& vchPubKey, bool fSkipCheck);
};

struct CExtKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    CKey key;

    const CKey& GetKey() const { return key; };
    CKey& GetMutableKey() { return key; };

    friend bool operator==(const CExtKey& a, const CExtKey& b)
    {
        return a.nDepth == b.nDepth &&
            memcmp(&a.vchFingerprint[0], &b.vchFingerprint[0], sizeof(vchFingerprint)) == 0 &&
            a.nChild == b.nChild &&
            a.chaincode == b.chaincode &&
            a.key == b.key;
    }

    void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
    bool Derive(CExtKey& out, unsigned int nChild) const;
    CExtPubKey Neuter() const;
    void SetMaster(const std::vector<unsigned char>& hashkey, const unsigned char* seed, unsigned int nSeedLen);
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        unsigned int len = BIP32_EXTKEY_SIZE;
        ::WriteCompactSize(s, len);
        unsigned char code[BIP32_EXTKEY_SIZE];
        Encode(code);
        s.write((const char *)&code[0], len);
    }
    template <typename Stream>
    void Unserialize(Stream& s)
    {
        unsigned int len = ::ReadCompactSize(s);
        unsigned char code[BIP32_EXTKEY_SIZE];
        s.read((char *)&code[0], len);
        Decode(code);
    }
};

/** Initialize the elliptic curve support. May not be called twice without calling ECC_Stop first. */
void ECC_Start(void);

/** Deinitialize the elliptic curve support. No-op if ECC_Start wasn't called first. */
void ECC_Stop(void);

/** Check that required EC support is available at runtime. */
bool ECC_InitSanityCheck(void);

#endif // GULDEN_KEY_H
