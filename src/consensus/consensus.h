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

#ifndef GULDEN_CONSENSUS_CONSENSUS_H
#define GULDEN_CONSENSUS_CONSENSUS_H

#include <stdint.h>

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 1000000;
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
static const unsigned int MAX_BLOCK_WEIGHT = MAX_BLOCK_SERIALIZED_SIZE;
/** The maximum allowed size for a block excluding witness data, in bytes (network rule) */
static const unsigned int MAX_BLOCK_BASE_SIZE = MAX_BLOCK_SERIALIZED_SIZE;
/** The maximum allowed number of signature check operations in a block (network rule) */
static const int64_t MAX_BLOCK_SIGOPS_COST = 80000;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY_MAINNET = 100;
static const int COINBASE_MATURITY_MAINNET_PHASE4 = 1200;
static const int COINBASE_MATURITY_TESTNET = 10;
static const int COINBASE_MATURITY_TESTNET_PHASE4 = 40;

#define COINBASE_MATURITY (IsArgSet("-testnet") ? COINBASE_MATURITY_TESTNET : COINBASE_MATURITY_MAINNET)
#define COINBASE_MATURITY_PHASE4 (IsArgSet("-testnet") ? COINBASE_MATURITY_TESTNET_PHASE4 : COINBASE_MATURITY_MAINNET_PHASE4)

/** Flags for nSequence and nLockTime locks */
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTimePast() instead of nTime for end point timestamp. */
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 1),
};

#endif // GULDEN_CONSENSUS_CONSENSUS_H
