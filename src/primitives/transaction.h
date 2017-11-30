// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_TRANSACTION_H
#define BITCOIN_PRIMITIVES_TRANSACTION_H

#include "amount.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"

//Gulden
#include "pubkey.h"
#include "streams.h"
#include "utilstrencodings.h"
#include <new> // Required for placement 'new'.
#include <bitset>


static const int SERIALIZE_TRANSACTION_NO_WITNESS = 0x40000000;

static const int WITNESS_SCALE_FACTOR = 4;

inline bool IsOldTransactionVersion(const unsigned int nVersion) { return nVersion < 4 || nVersion > 10000; }

struct CBlockPosition
{
    uint64_t blockNumber; // Position of block on blockchain that contains our transaction.
    uint64_t transactionIndex; // Position of transaction within the block.
    CBlockPosition(uint64_t blockNumber_, uint64_t transactionIndex_) : blockNumber(blockNumber_), transactionIndex(transactionIndex_) {}

    friend bool operator<(const CBlockPosition& a, const CBlockPosition& b)
    {
        if (a.blockNumber < b.blockNumber)
            return true;
        if (a.blockNumber == b.blockNumber && a.transactionIndex < b.transactionIndex)
            return true;

        return false;
    }

    friend bool operator==(const CBlockPosition& a, const CBlockPosition& b)
    {
        return (a.blockNumber == b.blockNumber && a.transactionIndex == b.transactionIndex);
    }
};


// Represented in class as 5 bits - so maximum of 32 values
enum CTxInType : uint8_t
{
    //fixme: (GULDEN) (2.0) What types do we even need here?
};

// Only 3 bits available for TxInFlags (used as bit flags so only 3 values)
enum CTxInFlags : uint8_t
{
    //fixme: NEXTNEXTNEXT HIGHHIGHHIGH - Implement these two.
    IndexBasedOutpoint,  // Outpoint is an index instead of a hash
    HasSequenceNumber,
    CTxInFutureFlag2
};

#define UINT31_MAX 2147483647

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    // fixme: (GULDEN) (2.1) (MED) - We can reduce memory consumption here by using something like prevector for hash cases.
    // Outpoint either uses hash or 'block position' never both.
    union
    {
        uint256 hash;
        CBlockPosition prevBlock;
    };
    uint32_t isHash: 1; // Set to 0 when using prevBlock, 1 when using hash.
    uint32_t n : 31;

    COutPoint(): hash(uint256()), isHash(1), n(UINT31_MAX) { }
    COutPoint(const uint256& hashIn, uint32_t nIn): hash(hashIn), isHash(1), n(nIn) { }
    COutPoint(const uint64_t blockNumber, const uint64_t transactionIndex, uint32_t nIn): prevBlock(blockNumber, transactionIndex), isHash(0), n(nIn) { }

    template <typename Stream> inline void ReadFromStream(Stream& s, CTxInType nType, uint8_t nFlags, int nTransactionVersion)
    {
        const CSerActionUnserialize ser_action;

        if (IsOldTransactionVersion(nTransactionVersion))
        {
            isHash = 1;
            STRREAD(hash);
            uint32_t n_;
            STRREAD(n_);
            n = n_;
        }
        else
        {
            std::bitset<3> flags(nFlags);
            if (!flags[IndexBasedOutpoint])
            {
                isHash = 1;
                STRREAD(hash);
            }
            else
            {
                isHash = 0;
                STRREAD(VARINT(prevBlock.blockNumber));
                STRREAD(VARINT(prevBlock.transactionIndex));
            }
            uint32_t n_;
            STRREAD(VARINT(n_));
            n = n_;
        }
    }

    template <typename Stream> inline void WriteToStream(Stream& s, CTxInType nType, uint8_t nFlags, int nTransactionVersion) const
    {
        const CSerActionSerialize ser_action;

        if (IsOldTransactionVersion(nTransactionVersion))
        {
            STRWRITE(hash);
            uint32_t nTemp = (n == UINT31_MAX ? std::numeric_limits<uint32_t>::max() : (uint32_t)n);
            STRWRITE(nTemp);
        }
        else
        {
            std::bitset<3> flags(nFlags);
            if (!flags[IndexBasedOutpoint])
            {
                STRWRITE(hash);
            }
            else
            {
                STRWRITE(VARINT(prevBlock.blockNumber));
                STRWRITE(VARINT(prevBlock.transactionIndex));
            }
            uint32_t n_ = n;
            STRWRITE(VARINT(n_));
        }
    }

    void SetNull() { hash.SetNull(); n = UINT31_MAX; }
    bool IsNull() const { return (hash.IsNull() && n == UINT31_MAX); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        if (a.isHash < b.isHash)
            return true;
        if (a.isHash)
        {
            int cmp = a.hash.Compare(b.hash);
            return cmp < 0 || (cmp == 0 && a.n < b.n);
        }
        else
        {
            return a.prevBlock < b.prevBlock;
        }
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.isHash == b.isHash && ((a.isHash && a.hash == b.hash) || (!a.isHash && a.prevBlock == b.prevBlock)) && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    // First 5 bits are type, last 3 bits are flags.
    uint8_t nTypeAndFlags;
    //fixme: gcc - future - In an ideal world we would just have nType be of type 'CTxOutType' - however GCC spits out unavoidable warnings when using an enum as part of a bitfield, so we use these getter/setter methods to work around it.
    CTxInType GetType() const
    {
        return (CTxInType) ( (nTypeAndFlags & 0b11111000) >> 3 );
    }
    CTxInFlags GetFlags() const
    {
        return (CTxInFlags) ( nTypeAndFlags & 0b00000111 );
    }
    bool FlagIsSet(CTxInFlags flag) const
    {
        return (GetFlags() & flag) != 0;
    }
    COutPoint prevout;
    CScript scriptSig;
    uint32_t nSequence;
    CScriptWitness scriptWitness; //! Only serialized through CTransaction

    /* Setting nSequence to this value for every input in a transaction
     * disables nLockTime. */
    static const uint32_t SEQUENCE_FINAL = 0xffffffff;

    /* Below flags apply in the context of BIP 68*/
    /* If this flag set, CTxIn::nSequence is NOT interpreted as a
     * relative lock-time. */
    static const uint32_t SEQUENCE_LOCKTIME_DISABLE_FLAG = (1 << 31);

    /* If CTxIn::nSequence encodes a relative lock-time and this flag
     * is set, the relative lock-time has units of 512 seconds,
     * otherwise it specifies blocks with a granularity of 1. */
    static const uint32_t SEQUENCE_LOCKTIME_TYPE_FLAG = (1 << 22);

    /* If CTxIn::nSequence encodes a relative lock-time, this mask is
     * applied to extract that lock-time from the sequence field. */
    static const uint32_t SEQUENCE_LOCKTIME_MASK = 0x0000ffff;

    /* In order to use the same number of bits to encode roughly the
     * same wall-clock duration, and because blocks are naturally
     * limited to occur every 600s on average, the minimum granularity
     * for time-based relative lock-time is fixed at 512 seconds.
     * Converting from CTxIn::nSequence to seconds is performed by
     * multiplying by 512 = 2^9, or equivalently shifting up by
     * 9 bits. */
    static const int SEQUENCE_LOCKTIME_GRANULARITY = 9;

    CTxIn()
    {
        nSequence = SEQUENCE_FINAL;
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);
    CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);

    template <typename Stream> inline void ReadFromStream(Stream& s, int nTransactionVersion)
    {
        const CSerActionUnserialize ser_action;

        //2.0 onwards we have versioning for CTxIn
        if (!IsOldTransactionVersion(nTransactionVersion))
        {
            uint8_t nTypeAndFlags_;
            STRREAD(nTypeAndFlags_);

            prevout.ReadFromStream(s, GetType(), GetFlags(), nTransactionVersion);
            //scriptSig is no longer used - everything goes in scriptWitness.
            if (FlagIsSet(HasSequenceNumber))
            {
                s >> VARINT(nSequence);
            }
        }
        else
        {
            prevout.ReadFromStream(s, GetType(), GetFlags(), nTransactionVersion);
            STRREAD(*(CScriptBase*)(&scriptSig));
            STRREAD(nSequence);
        }
    }
    template <typename Stream> inline void WriteToStream(Stream& s, int nTransactionVersion) const
    {
        const CSerActionSerialize ser_action;

        if (!IsOldTransactionVersion(nTransactionVersion))
        {
            uint8_t nTypeAndFlags_=0;
            STRWRITE(nTypeAndFlags_);

            prevout.WriteToStream(s, GetType(), GetFlags(), nTransactionVersion);

            if (FlagIsSet(HasSequenceNumber))
            {
                s << VARINT(nSequence);
            }
        }
        else
        {
            prevout.WriteToStream(s, GetType(), GetFlags(), nTransactionVersion);
            STRWRITE(*(CScriptBase*)(&scriptSig));
            STRWRITE(nSequence);
        }
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

enum CTxOutType : uint8_t
{
    //General purpose output types start from 0 counting upward
    ScriptLegacyOutput = 0,
    ScriptOutput = 1,

    //Specific/fixed purpose output types start from max counting backwards
    PoW2WitnessOutput = 31,
    StandardKeyHashOutput = 30
};


class CTxOutPoW2Witness
{
public:
    CKeyID spendingKeyID;
    CKeyID witnessKeyID;
    uint64_t lockFromBlock;
    uint64_t lockUntilBlock;
    uint64_t failCount;

    CTxOutPoW2Witness() {clear();}

    void clear()
    {
        spendingKeyID.SetNull();
        witnessKeyID.SetNull();
        lockFromBlock = 0;
        lockUntilBlock = 0;
        failCount = 0;
    }

    bool operator==(const CTxOutPoW2Witness& compare) const
    {
        return spendingKeyID == compare.spendingKeyID &&
               witnessKeyID == compare.witnessKeyID &&
               lockFromBlock == compare.lockFromBlock &&
               lockUntilBlock == compare.lockUntilBlock &&
               failCount == compare.failCount;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(spendingKeyID);
        READWRITE(witnessKeyID);
        READWRITE(lockFromBlock);
        READWRITE(lockUntilBlock);
        READWRITE(failCount);
    }
};

class CTxOutStandardKeyHash
{
public:
    CKeyID keyID;

    CTxOutStandardKeyHash(const CKeyID  keyID_) : keyID(keyID_) {}
    CTxOutStandardKeyHash() { clear(); }

    void clear()
    {
        keyID.SetNull();
    }

    bool operator==(const CTxOutStandardKeyHash& compare) const
    {
        return keyID == compare.keyID;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(keyID);
    }
};

/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    //fixme: gcc - future - In an ideal world we would just have nType be of type 'CTxOutType' - however GCC spits out unavoidable warnings when using an enum as part of a bitfield, so we use these getter/setter methods to work around it.
    CTxOutType GetType() const
    {
        return (CTxOutType)output.nType;
    }
    std::string GetTypeAsString() const
    {
        switch(CTxOutType(output.nType))
        {
            case CTxOutType::ScriptLegacyOutput:
            case CTxOutType::ScriptOutput:
                return "SCRIPT";
            case CTxOutType::PoW2WitnessOutput:
                return "POW2WITNESS";
            case CTxOutType::StandardKeyHashOutput:
                return "STANDARDKEYHASH";
        }
        return "";
    }
    void SetType(CTxOutType nType_)
    {
        output.DeleteOutput();
        output.nType = nType_;
        output.AllocOutput();
    }

    CAmount nValue;

    //Size of CTxOut in memory is very important - so we use a union here to try minimise the space taken.
    struct output
    {
        uint8_t nType: 5;
        uint8_t nValueBase: 3;
        union
        {
            CScript scriptPubKey;
            CTxOutPoW2Witness witnessDetails;
            CTxOutStandardKeyHash standardKeyHash;
        };
        output()
        {
            new(&scriptPubKey) CScript();
        }
        ~output()
        {
        }

        void DeleteOutput()
        {
            switch(nType)
            {
                case CTxOutType::ScriptLegacyOutput:
                case CTxOutType::ScriptOutput:
                {
                    scriptPubKey.clear();
                    scriptPubKey.~CScript(); break;
                }
                case CTxOutType::PoW2WitnessOutput:
                {
                    witnessDetails.clear();
                    witnessDetails.~CTxOutPoW2Witness(); break;
                }
                case CTxOutType::StandardKeyHashOutput:
                {
                    standardKeyHash.clear();
                    standardKeyHash.~CTxOutStandardKeyHash(); break;
                }
            }
        }

        void AllocOutput()
        {
            switch(nType)
            {
                case CTxOutType::ScriptLegacyOutput:
                case CTxOutType::ScriptOutput:
                    new(&scriptPubKey) CScript(); break;
                case CTxOutType::PoW2WitnessOutput:
                    new(&witnessDetails) CTxOutPoW2Witness(); break;
                case CTxOutType::StandardKeyHashOutput:
                    new(&standardKeyHash) CTxOutStandardKeyHash(); break;
            }
        }

        template <typename Stream, typename Operation>
        inline void SerializeOp(Stream& s, Operation ser_action)
        {
            switch(nType)
            {
                case CTxOutType::ScriptLegacyOutput:
                case CTxOutType::ScriptOutput:
                    READWRITE(*(CScriptBase*)(&scriptPubKey)); break;
                case CTxOutType::PoW2WitnessOutput:
                    READWRITE(witnessDetails); break;
                case CTxOutType::StandardKeyHashOutput:
                    READWRITE(standardKeyHash); break;
            }
        }

        std::string GetHex(CTxOutType nType) const
        {
            std::vector<unsigned char> serData;
            {
                CVectorWriter serialisedWitnessHeaderInfoStream(SER_NETWORK, INIT_PROTO_VERSION, serData, 0);
                const_cast<output*>(this)->SerializeOp(serialisedWitnessHeaderInfoStream, CSerActionSerialize());
            }
            return HexStr(serData);
        }

        bool operator==(const output& compare) const
        {
            if (nType != compare.nType)
                return false;

            switch(CTxOutType(nType))
            {
                case CTxOutType::ScriptLegacyOutput:
                case CTxOutType::ScriptOutput:
                    return scriptPubKey == compare.scriptPubKey;
                case CTxOutType::PoW2WitnessOutput:
                    return witnessDetails == compare.witnessDetails;
                case CTxOutType::StandardKeyHashOutput:
                    return standardKeyHash == compare.standardKeyHash;
            }
            return false;
        }
    } output;


    bool IsUnspendable() const
    {
        if (GetType() <= CTxOutType::ScriptOutput)
            return output.scriptPubKey.IsUnspendable();

        //fixme: (GULDEN) (2.0) - Can our 'standard' outputs still be unspendable?
        return false;
    }

    virtual ~CTxOut()
    {
        //fixme: (GULDEN) (2.0) (IMPLEMENT)
        output.DeleteOutput();
    }

    CTxOut operator=(const CTxOut& copyFrom)
    {
        SetType(CTxOutType(copyFrom.output.nType));
        output.nValueBase = copyFrom.output.nValueBase;
        nValue = copyFrom.nValue;
        switch(CTxOutType(output.nType))
        {
            case CTxOutType::ScriptLegacyOutput:
            case CTxOutType::ScriptOutput:
                output.scriptPubKey = copyFrom.output.scriptPubKey; break;
            case CTxOutType::PoW2WitnessOutput:
                output.witnessDetails = copyFrom.output.witnessDetails; break;
            case CTxOutType::StandardKeyHashOutput:
                output.standardKeyHash = copyFrom.output.standardKeyHash; break;
        }
        return *this;
    }

    CTxOut(const CTxOut& copyFrom)
    {
        SetType(CTxOutType(copyFrom.output.nType));
        nValue = copyFrom.nValue;
        output.nValueBase = copyFrom.output.nValueBase;
        switch(CTxOutType(output.nType))
        {
            case CTxOutType::ScriptLegacyOutput:
            case CTxOutType::ScriptOutput:
                output.scriptPubKey = copyFrom.output.scriptPubKey; break;
            case CTxOutType::PoW2WitnessOutput:
                output.witnessDetails = copyFrom.output.witnessDetails; break;
            case CTxOutType::StandardKeyHashOutput:
                output.standardKeyHash = copyFrom.output.standardKeyHash; break;
        }
    }

    CTxOut(CTxOut&& copyFrom)
    {
        SetType(CTxOutType(copyFrom.output.nType));
        nValue = copyFrom.nValue;
        output.nValueBase = copyFrom.output.nValueBase;
        switch(CTxOutType(output.nType))
        {
            case CTxOutType::ScriptLegacyOutput:
            case CTxOutType::ScriptOutput:
                output.scriptPubKey = copyFrom.output.scriptPubKey; break;
            case CTxOutType::PoW2WitnessOutput:
                output.witnessDetails = copyFrom.output.witnessDetails; break;
            case CTxOutType::StandardKeyHashOutput:
                output.standardKeyHash = copyFrom.output.standardKeyHash; break;
        }
    }

    CTxOut& operator=(CTxOut&& copyFrom)
    {
        SetType(CTxOutType(copyFrom.output.nType));
        nValue = copyFrom.nValue;
        output.nValueBase = copyFrom.output.nValueBase;
        switch(CTxOutType(output.nType))
        {
            case CTxOutType::ScriptLegacyOutput:
            case CTxOutType::ScriptOutput:
                output.scriptPubKey = copyFrom.output.scriptPubKey; break;
            case CTxOutType::PoW2WitnessOutput:
                output.witnessDetails = copyFrom.output.witnessDetails; break;
            case CTxOutType::StandardKeyHashOutput:
                output.standardKeyHash = copyFrom.output.standardKeyHash; break;
        }
        return *this;
    }

    CTxOut()
    {
        SetNull();
    }

    CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        if (ser_action.ForRead())
        {
            // Test whether we are dealing with the new or old transaction format by reading in 64 bits and testing them.
            // If the 10 leading bits are zero or the stream does not contain 64 bits
            // a) Old transaction format should always have 10 most significant bits as zero when reading the initial 64 bit value that it starts with.
            // Largest output is 170'000'000'00000000 to have existed as this number greatly exceeds total supply at the point in time when old transactions existed. This number only occupies 5 bytes of a 64 bit integer....
            // b) Old transaction format must always have >8 bytes as it starts with 64 bit amount.
            try
            {
                STRPEEK(nValue);
                if (nValue & 0b1111111111000000000000000000000000000000000000000000000000000000)
                {
                    nValue = -1;
                }
                else
                {
                    //fixme: SBSU - Ideally we should simply 'skip' here instead of incurring read overhead, unfortunately the behaviour of CHashVerifier makes this complicated
                    //STRSKIP(nValue);
                    STRREAD(nValue);
                }
            }
            catch (std::ios_base::failure& e)
            {
                nValue = -1;
            }

            // Read in the new transaction format value, which is specified in a format that is much more compact in most circumstances.
            if (nValue == -1)
            {
                uint8_t nTypeAndValueBase;
                STRREAD(nTypeAndValueBase);
                output.nValueBase = (nTypeAndValueBase & 0b00000111);
                SetType(CTxOutType((nTypeAndValueBase & 0b11111000) >> 3));

                assert(output.nType != CTxOutType::ScriptLegacyOutput);

                STRREAD(VARINT(nValue)); // Compacted value is stored as a varint.
                switch(output.nValueBase) // Which further needs to be multiplied by base to get the full int64 value.
                {
                    case 0: break;                          // 8 decimal precision  (0.00000008, 87654321.12345678 etc.)
                    case 1: nValue *= 100; break;           // 6 decimal precision  (0.00000600, 87654321.12345600 etc.)
                    case 2: nValue *= 10000; break;         // 4 decimal precision  (0.00040000, 87654321.12340000 etc.)
                    case 3: nValue *= 1000000; break;       // 2 decimal precision  (0.02000000, 87654321.12000000 etc.)
                    case 4: nValue *= 10000000; break;      // 1 decimal precision  (0.10000000, 87654321.10000000 etc.)
                    case 5: nValue *= 100000000; break;     // 1 significant digit precision (1.00000000, 87654321.00000000 etc.)
                    case 6: nValue *= 10000000000; break;   // 3 significant digit precision (200.00000000, 876543200.00000000 etc.)
                    case 7: nValue *= 1000000000000; break; // 5 significant digit precision (40000.00000000, 876540000.00000000 etc.)
                };
            }
        }
        else
        {
            if (GetType() == CTxOutType::ScriptLegacyOutput)
            {
                // Old transaction format.
                STRWRITE(nValue);
            }
            else
            {
                CAmount nValueWrite = nValue;

                // If we don't already have a 'base' set calculate the best one.
                if (output.nValueBase == 0)
                {
                    //fixme: (Gulden) - Is there some 'trick' to calculate this faster without so much branching?
                    if (nValue % 1000000000000 == 0)    { output.nValueBase = 7; } // 4 significant digit precision
                    else if (nValue % 10000000000 == 0) { output.nValueBase = 6; } // 2 significant digit precision
                    else if (nValue % 100000000 == 0)   { output.nValueBase = 5; } // 1 significant digit precision
                    else if (nValue % 10000000 == 0)    { output.nValueBase = 4; } // 1 decimal precision
                    else if (nValue % 1000000 == 0)     { output.nValueBase = 3; } // 2 decimal precision
                    else if (nValue % 10000 == 0)       { output.nValueBase = 2; } // 4 decimal precision
                    else if (nValue % 100 == 0)         { output.nValueBase = 1; } // 6 decimal precision
                    // 8 decimal precision.
                }
                // Adjust nValueWrite to the new base.
                switch (output.nValueBase)
                {
                    case 7: nValueWrite /= 1000000000000; break;
                    case 6: nValueWrite /= 10000000000; break;
                    case 5: nValueWrite /= 100000000; break;
                    case 4: nValueWrite /= 10000000; break;
                    case 3: nValueWrite /= 1000000; break;
                    case 2: nValueWrite /= 10000; break;
                    case 1: nValueWrite /= 100; break;
                }

                uint8_t nTypeAndValueBase = 0;
                nTypeAndValueBase = output.nType;
                nTypeAndValueBase <<= 3;
                nTypeAndValueBase |= output.nValueBase;
                STRWRITE(nTypeAndValueBase);
                STRWRITE(VARINT(nValueWrite));
            }
        }
        output.SerializeOp(s, ser_action);
    }

    void SetNull()
    {
        SetType(ScriptLegacyOutput);
        output.nValueBase = 0;
        nValue = -1;
        switch(output.nType)
        {
            case CTxOutType::ScriptLegacyOutput:
            case CTxOutType::ScriptOutput:
                output.scriptPubKey.clear(); break;
            case CTxOutType::PoW2WitnessOutput:
                output.witnessDetails.clear(); break;
            case CTxOutType::StandardKeyHashOutput:
                output.standardKeyHash.clear(); break;
        }
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue == b.nValue &&
                a.output == b.output);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

struct CMutableTransaction;

#define UnserializeTransaction UnserializeTransactionOld
#define SerializeTransaction SerializeTransactionOld
/**
 * Basic transaction serialization format:
 * - int32_t nVersion
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - uint32_t nLockTime
 *
 * Extended transaction serialization format:
 * - int32_t nVersion
 * - unsigned char dummy = 0x00
 * - unsigned char flags (!= 0)
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - if (flags & 1):
 *   - CTxWitness wit;
 * - uint32_t nLockTime
 */
template<typename Stream, typename TxType>
inline void UnserializeTransaction(TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s >> tx.nVersion;
    unsigned char flags = 0;
    tx.vin.clear();
    tx.vout.clear();
    /* Try to read the vin. In case the dummy is there, this will be read as an empty vector. */
    uint64_t nSize;
    s >> COMPACTSIZE(nSize);
    tx.vin.resize(nSize);
    for (auto& in : tx.vin)
    {
        in.ReadFromStream(s, tx.nVersion);
    }
    if (tx.vin.size() == 0 && fAllowWitness) {
        /* We read a dummy or an empty vin. */
        s >> flags;
        if (flags != 0) {
            uint64_t nSize;
            s >> COMPACTSIZE(nSize);
            tx.vin.resize(nSize);
            for (auto& in : tx.vin)
            {
                in.ReadFromStream(s, tx.nVersion);
            }
            s >> COMPACTSIZEVECTOR(tx.vout);
        }
    } else {
        /* We read a non-empty vin. Assume a normal vout follows. */
        s >> COMPACTSIZEVECTOR(tx.vout);
    }
    if ((flags & 1) && fAllowWitness) {
        /* The witness flag is present, and we support witnesses. */
        flags ^= 1;
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s >> COMPACTSIZEVECTOR(tx.vin[i].scriptWitness.stack);
        }
    }
    if (flags) {
        /* Unknown flag in the serialization */
        throw std::ios_base::failure("Unknown transaction optional data");
    }
    s >> tx.nLockTime;
}

template<typename Stream, typename TxType>
inline void SerializeTransaction(const TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s << tx.nVersion;
    unsigned char flags = 0;
    // Consistency check
    if (fAllowWitness) {
        /* Check whether witnesses need to be serialized. */
        if (tx.HasWitness()) {
            flags |= 1;
        }
    }
    if (flags) {
        /* Use extended format in case witnesses are to be serialized. */
        std::vector<CTxIn> vinDummy;
        s << COMPACTSIZE(vinDummy.size());
        //vinDummy[0].WriteToStream(s, tx.nVersion);
        s << flags;
    }
    s << COMPACTSIZE(tx.vin.size());
    for (const auto& in : tx.vin)
    {
        in.WriteToStream(s, tx.nVersion);
    }
    s << COMPACTSIZEVECTOR(tx.vout);
    if (flags & 1) {
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s << COMPACTSIZEVECTOR(tx.vin[i].scriptWitness.stack);
        }
    }
    s << tx.nLockTime;
}
#undef UnserializeTransaction
#undef SerializeTransaction

//New transaction format:
//Version number: CVarInt [1 byte] (but forward compat for larger sizes)
//Flags: bitset [1 byte] (7 bytes used, 8th byte signals ExtraFlags)  {Current flags are for 1/2/3 input transactions, 1/2/3 output transactions and locktime}
//ExtraFlags: bitset [1 byte] (but never currently present, only there for forwards compat)
//Input count: CVarInt [0-9 byte] (only present in the event that input count flags are all unset)
//Vector of inputs
//Output count: CVarInt [0-9 byte] (only present in the event that output count flags are all unset)
//Vector of outputs
//Vector of witnesses (no size)
//Lock time: CVarInt [0-9 byte] (only present if locktime flag is set)

//>90% of transactions involve either 1/2/3 inputs or 1/2/3 outputs.
enum TransactionFlags : uint8_t
{
    HasOneInput,
    HasTwoInputs,
    HasThreeInputs,
    HasOneOutput,
    HasTwoOutputs,
    HasThreeOutputs,
    HasLockTime,
    HasExtraFlags
};

template<typename Stream, typename TxType> inline void SerializeTransaction(const TxType& tx, Stream& s) {
    if (IsOldTransactionVersion(tx.nVersion))
        return SerializeTransactionOld(tx, s);

    // Setup flags
    switch(tx.vin.size())
    {
        case 1:
            tx.flags.set(HasOneInput, true).set(HasTwoInputs, false).set(HasThreeInputs, false); break;
        case 2:
            tx.flags.set(HasOneInput, false).set(HasTwoInputs, true).set(HasThreeInputs, false); break;
        case 3:
            tx.flags.set(HasOneInput, false).set(HasTwoInputs, false).set(HasThreeInputs, true); break;
        default:
            tx.flags.set(HasOneInput, false).set(HasTwoInputs, false).set(HasThreeInputs, false);
    }
    switch(tx.vout.size())
    {
        case 1:
            tx.flags.set(HasOneOutput, true).set(HasTwoOutputs, false).set(HasThreeOutputs, false); break;
        case 2:
            tx.flags.set(HasOneOutput, false).set(HasTwoOutputs, true).set(HasThreeOutputs, false); break;
        case 3:
            tx.flags.set(HasOneOutput, false).set(HasTwoOutputs, false).set(HasThreeOutputs, true); break;
        default:
            tx.flags.set(HasOneOutput, false).set(HasTwoOutputs, false).set(HasThreeOutputs, false);
    }
    tx.flags.set(HasLockTime, false);
    if (tx.nLockTime > 0)
    {
        tx.flags.set(HasLockTime, true);
    }

    //Serialization begins.
    //Version
    s << VARINT(tx.nVersion);

    //Flags + (opt) ExtraFlags
    s << static_cast<uint8_t>(tx.flags.to_ulong());
    if(tx.flags[HasExtraFlags])
        s << static_cast<uint8_t>(tx.extraFlags.to_ulong());

    //(opt) Input count + Inputs
    if ( !(tx.flags[HasOneInput] || tx.flags[HasTwoInputs] || tx.flags[HasThreeInputs]) )
        s << VARINT(tx.vin.size());
    for (const auto& in : tx.vin)
    {
        in.WriteToStream(s, tx.nVersion);
    }

    //(opt) Output count + Outputs
    if (tx.flags[HasOneOutput] || tx.flags[HasTwoOutputs] || tx.flags[HasThreeOutputs])
        s << NOSIZEVECTOR(tx.vout);
    else
        s << VARINTVECTOR(tx.vout);

    //Witness data
    if (!(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS)) {
        for (size_t i = 0; i < tx.vin.size(); i++)
        {
            s << VARINTVECTOR(tx.vin[i].scriptWitness.stack);
        }
    }

    // (opt) lock time
    if (tx.flags[HasLockTime])
    {
        s << VARINT(tx.nLockTime);
    }
}


template<typename Stream, typename TxType>
inline void UnserializeTransaction(TxType& tx, Stream& s) {
    const CSerActionUnserialize ser_action;

    //Version
    STRPEEK(tx.nVersion);
    if (IsOldTransactionVersion(tx.nVersion))
    {
        UnserializeTransactionOld(tx, s);
        return;
    }
    tx.nVersion = ReadVarInt<Stream, int32_t>(s);

    //Flags + (opt) ExtraFlags
    tx.flags.reset();
    tx.extraFlags.reset();
    unsigned char cFlags, cExtraFlags;
    s >> cFlags;
    tx.flags = std::bitset<8>(cFlags);
    if (tx.flags[HasExtraFlags])
    {
        s >> cExtraFlags;
        tx.extraFlags = std::bitset<8>(cExtraFlags);
    }

    //(opt) Input count + Inputs
    tx.vin.clear();
    if (tx.flags[HasOneInput])
    {
        tx.vin.resize(1);
    }
    else if (tx.flags[HasTwoInputs])
    {
        tx.vin.resize(2);
    }
    else if (tx.flags[HasThreeInputs])
    {
        tx.vin.resize(3);
    }
    else
    {
        int nSize;
        s >> VARINT(nSize);
        tx.vin.resize(nSize);
    }
    for (auto & txIn : tx.vin)
    {
        txIn.ReadFromStream(s, tx.nVersion);
    }

    //(opt) Output count + Outputs
    tx.vout.clear();
    if (tx.flags[HasOneOutput])
    {
        tx.vout.resize(1);
        s >> NOSIZEVECTOR(tx.vout);
    }
    else if (tx.flags[HasTwoOutputs])
    {
        tx.vout.resize(2);
        s >> NOSIZEVECTOR(tx.vout);
    }
    else if (tx.flags[HasThreeOutputs])
    {
        tx.vout.resize(3);
        s >> NOSIZEVECTOR(tx.vout);
    }
    else
    {
        s >> VARINTVECTOR(tx.vout);
    }

    if (!(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS)) {
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s >> VARINTVECTOR(tx.vin[i].scriptWitness.stack);
        }
    }

    // (opt) lock time
    if (tx.flags[HasLockTime])
    {
        tx.nLockTime = ReadVarInt<Stream, uint32_t>(s);
    }
    else
    {
        tx.nLockTime = 0;
    }
}


//fixme: (GULDEN) (2.1) Remove
#define CURRENT_TX_VERSION_POW2 (GetPoW2Phase(chainActive.Tip()->pprev, Params()) >= 4 ? CTransaction::SEGSIG_ACTIVATION_VERSION : CTransaction::CURRENT_VERSION)

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class CTransaction
{
public:
    // Default transaction version.
    static const int32_t CURRENT_VERSION=3;

    // Changing the default transaction version requires a two step process: first
    // adapting relay policy by bumping MAX_STANDARD_VERSION, and then later date
    // bumping the default CURRENT_VERSION at which point both CURRENT_VERSION and
    // MAX_STANDARD_VERSION will be equal.
    static const int32_t MAX_STANDARD_VERSION=4;
    static const int32_t SEGSIG_ACTIVATION_VERSION=4;

    // The local variables are made const to prevent unintended modification
    // without updating the cached hash value. However, CTransaction is not
    // actually immutable; deserialization and assignment are implemented,
    // and bypass the constness. This is safe, as they update the entire
    // structure, including the hash.
    const int32_t nVersion;
    const std::vector<CTxIn> vin;
    const std::vector<CTxOut> vout;
    const uint32_t nLockTime;
    mutable std::bitset<8> flags;
    mutable std::bitset<8> extraFlags;//Currently unused but present for forwards compat.

private:
    /** Memory only. */
    const uint256 hash;

    uint256 ComputeHash() const;

public:
    /** Construct a CTransaction that qualifies as IsNull() */
    CTransaction(int32_t nVersion_);

    /** Convert a CMutableTransaction into a CTransaction. */
    CTransaction(const CMutableTransaction &tx);
    CTransaction(CMutableTransaction &&tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }

    /** This deserializing constructor is provided instead of an Unserialize method.
     *  Unserialize is not possible, since it would require overwriting const fields. */
    template <typename Stream>
    CTransaction(deserialize_type, Stream& s) : CTransaction(CMutableTransaction(deserialize, s)) {}

    bool IsNull() const {
        return vin.empty() && vout.empty();
    }

    const uint256& GetHash() const {
        return hash;
    }

    // Compute a hash that includes both transaction and witness data
    uint256 GetWitnessHash() const;

    // Return sum of txouts.
    CAmount GetValueOut() const;
    // GetValueIn() is a method on CCoinsViewCache, because
    // inputs must be known to compute value in.

    /**
     * Get the total transaction size in bytes, including witness data.
     * "Total Size" defined in BIP141 and BIP144.
     * @return Total transaction size in bytes
     */
    unsigned int GetTotalSize() const;

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull()) || IsPoW2WitnessCoinBase();
    }

    //fixme: (GULDEN) (2.0) - check second vin is a witness transaction.
    bool IsPoW2WitnessCoinBase() const
    {
        return (vin.size() == 2 && vin[0].prevout.IsNull());
    }

    friend bool operator==(const CTransaction& a, const CTransaction& b)
    {
        return a.hash == b.hash;
    }

    friend bool operator!=(const CTransaction& a, const CTransaction& b)
    {
        return a.hash != b.hash;
    }

    std::string ToString() const;

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};

/** A mutable version of CTransaction. */
struct CMutableTransaction
{
    int32_t nVersion;
    std::vector<CTxIn> vin;
    std::vector<CTxOut> vout;
    uint32_t nLockTime;
    mutable std::bitset<8> flags;
    mutable std::bitset<8> extraFlags;//Currently unused but present for forwards compat.

    CMutableTransaction(int32_t nVersion_);
    CMutableTransaction(const CTransaction& tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }


    template <typename Stream>
    inline void Unserialize(Stream& s) {
        UnserializeTransaction(*this, s);
    }

    template <typename Stream>
    CMutableTransaction(deserialize_type, Stream& s) {
        Unserialize(s);
    }

    /** Compute the hash of this CMutableTransaction. This is computed on the
     * fly, as opposed to GetHash() in CTransaction, which uses a cached result.
     */
    uint256 GetHash() const;

    friend bool operator==(const CMutableTransaction& a, const CMutableTransaction& b)
    {
        return a.GetHash() == b.GetHash();
    }

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};

typedef std::shared_ptr<const CTransaction> CTransactionRef;
static inline CTransactionRef MakeTransactionRef(int32_t nVersion_) { return std::make_shared<const CTransaction>(nVersion_); }
template <typename Tx> static inline CTransactionRef MakeTransactionRef(Tx&& txIn) { return std::make_shared<const CTransaction>(std::forward<Tx>(txIn)); }

/** Compute the weight of a transaction, as defined by BIP 141 */
int64_t GetTransactionWeight(const CTransaction &tx);

#endif // BITCOIN_PRIMITIVES_TRANSACTION_H
