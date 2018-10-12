// Copyright (c) 2015-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING


#ifndef GULDEN_DIFF_COMMON_H
#define GULDEN_DIFF_COMMON_H

#ifdef __JAVA__
    #define fDebug false
    #define LogPrintf(...)
    #define BLOCK_TYPE Block
    #define BLOCK_TIME(block) block.getTimeSeconds()
    #define INDEX_TYPE StoredBlock
    #define INDEX_HEIGHT(block) block.getHeight()
    //fixme: (2.0.1) (Mobile)
    #define INDEX_TIME(block) block.getHeader().getTimeSeconds()
    #define INDEX_PREV(block) blockStore.get(block.getHeader().getPrevBlockHash())
    #define INDEX_TARGET(block) block.getHeader().getDifficultyTarget()
    #define DIFF_SWITCHOVER(T, M) (Constants.TEST ? T : M)
    #define DIFF_ABS Math.abs
    #define arith_uint256(x) BigInteger.valueOf(x)
    #define SET_COMPACT(EXPANDED, COMPACT) EXPANDED = Utils.decodeCompactBits(COMPACT)
    #define GET_COMPACT(EXPANDED) Utils.encodeCompactBits(EXPANDED)
    #define BIGINT_MULTIPLY(x, y) x.multiply(y)
    #define BIGINT_DIVIDE(x, y) x.divide(y)
    #define BIGINT_GREATER_THAN(x, y) (x.compareTo(y) == 1)
#else
    #include <consensus/params.h>
    #include <arith_uint256.h>
    #include <chain.h>
    #include <sync.h>
    #include <util.h>
    #include <stdint.h>
    #define BLOCK_TYPE CBlockHeader*
    #define BLOCK_TIME(block) block->nTime
    #define INDEX_TYPE CBlockIndex*
    #define INDEX_HEIGHT(block) block->nHeight
    #define INDEX_TIME(block) block->GetBlockTime()
    #define INDEX_PREV(block) block->pprev
    #define INDEX_TARGET(block) block->nBits
    #define DIFF_SWITCHOVER(TEST, MAIN) (IsArgSet("-testnet") ? TEST :  MAIN)
    #define DIFF_ABS std::abs
    #define SET_COMPACT(EXPANDED, COMPACT) EXPANDED.SetCompact(COMPACT)
    #define GET_COMPACT(EXPANDED) EXPANDED.GetCompact()
    #define BIGINT_MULTIPLY(x, y) x * y
    #define BIGINT_DIVIDE(x, y) x / y
    #define BIGINT_GREATER_THAN(x, y) (x > y)
#endif

#ifndef __JAVA__
#if defined __cplusplus
extern "C" {
#endif

extern unsigned int GetNextWorkRequired(const INDEX_TYPE indexLast, const BLOCK_TYPE block, unsigned int nPowTargetSpacing, unsigned int nPowLimit);

#if defined __cplusplus
};
#endif
#endif

#endif
