// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Centure developers
// All modifications:
// Copyright (c) 2016-2022 The Centure developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the Libre Chain License, see the accompanying
// file COPYING

#ifndef POLICY_POLICY_H
#define POLICY_POLICY_H

#include "consensus/consensus.h"
#include "feerate.h"
#include "script/interpreter.h"
#include "script/standard.h"

#include <string>

class CCoinsViewCache;
class CTxOut;

/** Default for -blockmaxsize, which controls the maximum size of block the mining code will create **/
static const unsigned int DEFAULT_BLOCK_MAX_SIZE = 250000;
/** Default for -blockmaxweight, which controls the range of block weights the mining code will create **/
static const unsigned int DEFAULT_BLOCK_MAX_WEIGHT = 3000000;
/** Default for -blockmintxfee, which sets the minimum feerate for a transaction in blocks created by mining code **/
static const unsigned int DEFAULT_BLOCK_MIN_TX_FEE = 1000;
/** The maximum weight for transactions we're willing to relay/mine */
static const unsigned int MAX_STANDARD_TX_WEIGHT = 400000;
/** Maximum number of signature check operations in an IsStandard() P2SH script */
static const unsigned int MAX_P2SH_SIGOPS = 15;
/** The maximum number of sigops we're willing to relay/mine in a single tx */
static const unsigned int MAX_STANDARD_TX_SIGOPS_COST = MAX_BLOCK_SIGOPS_COST/5;
/** Default for -maxmempool, maximum megabytes of mempool memory usage */
static const unsigned int DEFAULT_MAX_MEMPOOL_SIZE = 300;
static const unsigned int DEFAULT_MAX_MEMPOOL_SIZE_LOWMEM = 50;
/** Default for -incrementalrelayfee, which sets the minimum feerate increase for mempool limiting or BIP 125 replacement **/
static const unsigned int DEFAULT_INCREMENTAL_RELAY_FEE = 1000;
/** Default for -bytespersigop */
static const unsigned int DEFAULT_BYTES_PER_SIGOP = 20;

/** Min feerate for defining dust. Historically this has been the same as the
 * minRelayTxFee, however changing the dust limit changes which transactions are
 * standard and should be done with care and ideally rarely. It makes sense to
 * only increase the dust limit after prior releases were already not creating
 * outputs below the new threshold */
static const unsigned int DUST_RELAY_TX_FEE = 1000;
/**
 * Standard script verification flags that standard transactions will comply
 * with. However scripts violating these flags may still be present in valid
 * blocks and we must accept those blocks.
 */
static const unsigned int STANDARD_SCRIPT_VERIFY_FLAGS = MANDATORY_SCRIPT_VERIFY_FLAGS |
                                                         SCRIPT_VERIFY_DERSIG |
                                                         SCRIPT_VERIFY_STRICTENC |
                                                         SCRIPT_VERIFY_MINIMALDATA |
                                                         SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS |
                                                         SCRIPT_VERIFY_CLEANSTACK |
                                                         SCRIPT_VERIFY_MINIMALIF |
                                                         SCRIPT_VERIFY_NULLFAIL |
                                                         SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY |
                                                         SCRIPT_VERIFY_CHECKSEQUENCEVERIFY |
                                                         SCRIPT_VERIFY_LOW_S
                                                         ;

/** For convenience, standard but not mandatory verify flags. */
static const unsigned int STANDARD_NOT_MANDATORY_VERIFY_FLAGS = STANDARD_SCRIPT_VERIFY_FLAGS & ~MANDATORY_SCRIPT_VERIFY_FLAGS;

/** Used as the flags parameter to sequence and nLocktime checks in non-consensus code. */
static const unsigned int STANDARD_LOCKTIME_VERIFY_FLAGS = LOCKTIME_VERIFY_SEQUENCE |
                                                           LOCKTIME_MEDIAN_TIME_PAST;

CAmount GetDustThreshold(const CTxOut& txout, const CFeeRate& dustRelayFee);

bool IsDust(const CTxOut& txout, const CFeeRate& dustRelayFee);

bool IsStandard(const CScript& scriptPubKey, txnouttype& whichType, const bool segsigEnabled = false);
    /**
     * Check for standard transaction types
     * @return True if all outputs (scriptPubKeys) use only standard transaction forms
     */
bool IsStandardTx(const CTransaction& tx, std::string& reason, int nPoW2Version, const bool segsigEnabled = false);
    /**
     * Check for standard transaction types
     * @param[in] mapInputs    Map of previous transactions that have outputs we're spending
     * @return True if all inputs (scriptSigs) use only standard transaction forms
     */
bool AreInputsStandard(const CTransaction& tx, const CCoinsViewCache& mapInputs);

bool IsSegregatedSignatureDataStandard(const CTransaction& tx, const CCoinsViewCache& mapInputs);

extern CFeeRate incrementalRelayFee;
extern CFeeRate dustRelayFee;
extern unsigned int nBytesPerSigOp;

/** Compute the virtual transaction size (weight reinterpreted as bytes). */
int64_t GetVirtualTransactionSize(int64_t nWeight, int64_t nSigOpCost);
int64_t GetVirtualTransactionSize(const CTransaction& tx, int64_t nSigOpCost = 0);
int64_t GetVirtualTransactionSizeDiscounted(int64_t nWeight, uint64_t numCoinbaseInputs, int64_t nSigOpCost);
int64_t GetVirtualTransactionSizeDiscounted(const CTransaction& tx, uint64_t numCoinbaseInputs, int64_t nSigOpCost=0);

#endif
