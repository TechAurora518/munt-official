// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2017-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "guldenconsensus.h"

#include "primitives/transaction.h"
#include "pubkey.h"
#include "script/interpreter.h"
#include "version.h"
#include "validation.h"

namespace {

/** A class that deserializes a single CTransaction one time. */
class TxInputStream
{
public:
    TxInputStream(int nTypeIn, int nVersionIn, const unsigned char *txTo, size_t txToLen) :
    m_type(nTypeIn),
    m_version(nVersionIn),
    m_data(txTo),
    m_remaining(txToLen)
    {}

    void read(char* pch, size_t nSize)
    {
        if (nSize > m_remaining)
            throw std::ios_base::failure(std::string(__func__) + ": end of data");

        if (pch == NULL)
            throw std::ios_base::failure(std::string(__func__) + ": bad destination buffer");

        if (m_data == NULL)
            throw std::ios_base::failure(std::string(__func__) + ": bad source buffer");

        memcpy(pch, m_data, nSize);
        m_remaining -= nSize;
        m_data += nSize;
    }

    void peek(char* pch, size_t nSize)
    {
        if (nSize > m_remaining)
            throw std::ios_base::failure(std::string(__func__) + ": end of data");

        if (pch == NULL)
            throw std::ios_base::failure(std::string(__func__) + ": bad destination buffer");

        if (m_data == NULL)
            throw std::ios_base::failure(std::string(__func__) + ": bad source buffer");

        memcpy(pch, m_data, nSize);
    }

    void ignore(size_t nSize)
    {
        m_remaining -= nSize;
        m_data += nSize;
    }

    template<typename T>
    TxInputStream& operator>>(T& obj)
    {
        ::Unserialize(*this, obj);
        return *this;
    }

    int GetVersion() const { return m_version; }
    int GetType() const { return m_type; }
private:
    const int m_type;
    const int m_version;
    const unsigned char* m_data;
    size_t m_remaining;
};

inline int set_error(guldenconsensus_error* ret, guldenconsensus_error serror)
{
    if (ret)
        *ret = serror;
    return 0;
}

struct ECCryptoClosure
{
    ECCVerifyHandle handle;
};

ECCryptoClosure instance_of_eccryptoclosure;
}

/** Check that all specified flags are part of the libconsensus interface. */
static bool verify_flags(unsigned int flags)
{
    return (flags & ~(guldenconsensus_SCRIPT_FLAGS_VERIFY_ALL)) == 0;
}

static int verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, CAmount amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, guldenconsensus_error* err)
{
    if (!verify_flags(flags)) {
        return guldenconsensus_ERR_INVALID_FLAGS;
    }
    try {
        TxInputStream stream(SER_NETWORK, PROTOCOL_VERSION, txTo, txToLen);
        CTransaction tx(deserialize, stream);
        if (nIn >= tx.vin.size())
            return set_error(err, guldenconsensus_ERR_TX_INDEX);
        if (GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION) != txToLen)
            return set_error(err, guldenconsensus_ERR_TX_SIZE_MISMATCH);

        // Regardless of the verification result, the tx did not error.
        set_error(err, guldenconsensus_ERR_OK);

        PrecomputedTransactionData txdata(tx);
        //fixme: (2.0) (HIGH) - can't pass CKeyID(), need the actual keyID
        CKeyID tempKeyID;
        return VerifyScript(tx.vin[nIn].scriptSig, CScript(scriptPubKey, scriptPubKey + scriptPubKeyLen), &tx.vin[nIn].segregatedSignatureData, flags, TransactionSignatureChecker(tempKeyID, &tx, nIn, amount, txdata), NULL);
    } catch (const std::exception&) {
        return set_error(err, guldenconsensus_ERR_TX_DESERIALIZE); // Error deserializing
    }
}

int guldenconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, guldenconsensus_error* err)
{
    CAmount am(amount);
    return ::verify_script(scriptPubKey, scriptPubKeyLen, am, txTo, txToLen, nIn, flags, err);
}


int guldenconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                   const unsigned char *txTo        , unsigned int txToLen,
                                   unsigned int nIn, unsigned int flags, guldenconsensus_error* err)
{
    if (flags & guldenconsensus_SCRIPT_FLAGS_VERIFY_WITNESS) {
        return set_error(err, guldenconsensus_ERR_AMOUNT_REQUIRED);
    }

    CAmount am(0);
    return ::verify_script(scriptPubKey, scriptPubKeyLen, am, txTo, txToLen, nIn, flags, err);
}

unsigned int guldenconsensus_version()
{
    // Just use the API version for now
    return GULDENCONSENSUS_API_VER;
}
