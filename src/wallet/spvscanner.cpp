// Copyright (c) 2018 The Gulden developers
// Authored by: Willem de Jonge (willem@isnapp.nl)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "spvscanner.h"
#include "../net_processing.h"
#include "../validation/validation.h"
#include "wallet.h"

#include <algorithm>

// Maximum number of requests pending. This number should be high enough so that
// the requests can be reasonably distributed over our peers.
const static int nMaxPendingRequests = 512;

// Seconds before oldest key to start scanning blocks.
const static int64_t startTimeGap = 3 * 3600;

CSPVScanner::CSPVScanner(CWallet& _wallet) :
    wallet(_wallet),
    startTime(0)
{
    LOCK(cs_main);

    CWalletDB walletdb(*wallet.dbw);
    CBlockLocator locator;
    if (!walletdb.ReadLastSPVBlockProcessed(locator))
        locator = chainActive.GetLocatorPoW2(chainActive.Genesis());

    lastProcessed = FindForkInGlobalIndex(headerChain, locator);
    requestTip = lastProcessed;

    // fixme (SPV): get actual seed time, now for testing hacked to birth of test wallet
    // was uint64_t seedTime = wallet.GetOldestKeyPoolTime();
    int64_t seedTime = 1527681623; // 05/30/2018 @ 12:00pm (UTC)

    startTime =  std::max(int64_t(0), seedTime - startTimeGap);

    LogPrint(BCLog::WALLET, "Using %s (height = %d) as last processed SPV block\n",
             lastProcessed->GetBlockHashPoW2().ToString(), lastProcessed->nHeight);

    RegisterValidationInterface(this);
}

CSPVScanner::~CSPVScanner()
{
    UnregisterValidationInterface(this);
}

void CSPVScanner::StartScan()
{
    RequestBlocks();
}

void CSPVScanner::RequestBlocks()
{
    LOCK2(cs_main, wallet.cs_wallet);

    // put lastProcessed and/or requestTip back on chain if forked
    while (!headerChain.Contains(requestTip)) {
        if (requestTip->nHeight > lastProcessed->nHeight) {
            CancelPriorityDownload(requestTip);
            requestTip = requestTip->pprev;
        }
        else { // so here requestTip == lastProcessed
            std::shared_ptr<CBlock> pblock = std::make_shared<CBlock>();
            if (!ReadBlockFromDisk(*pblock, lastProcessed, Params())) {
                wallet.BlockDisconnected(pblock);
            }
            else {
                // This block must be on disk, it was processed before.
                // So pruning has to keep at least as many blocks back as the longest fork we are willing to handle.
                assert(false);
            }
            UpdateLastProcessed(lastProcessed->pprev);
            requestTip = lastProcessed;
        }
    }

    // skip blocks that are before startTime
    const CBlockIndex* skip = lastProcessed;
    while (skip->GetBlockTime() < startTime && headerChain.Height() > skip->nHeight)
        skip = headerChain.Next(skip);
    if (skip != lastProcessed) {
        LogPrint(BCLog::WALLET, "Skipping %d old blocks for SPV scan, up to height %d\n", skip->nHeight - lastProcessed->nHeight, skip->nHeight);
        UpdateLastProcessed(skip);
        if (lastProcessed->nHeight > requestTip->nHeight)
            requestTip = lastProcessed;
    }

    std::vector<const CBlockIndex*> blocksToRequest;

    // add requests for as long as nMaxPendingRequests is not reached and there are still heigher blocks in headerChain
    while (requestTip->nHeight - lastProcessed->nHeight < nMaxPendingRequests &&
           headerChain.Height() > requestTip->nHeight) {

        requestTip = headerChain.Next(requestTip);
        blocksToRequest.push_back(requestTip);
    }

    if (!blocksToRequest.empty()) {
        LogPrint(BCLog::WALLET, "Requesting %d blocks for SPV, up to height %d\n", blocksToRequest.size(), requestTip->nHeight);
        AddPriorityDownload(blocksToRequest);
    }
}

void CSPVScanner::ProcessPriorityRequest(const std::shared_ptr<const CBlock> &block, const CBlockIndex *pindex)
{
    LOCK2(cs_main, wallet.cs_wallet);

    // if chainActive is up-to-date no SPV blocks need to be requested, we can update SPV to the activeChain
    if (chainActive.Tip() == headerChain.Tip()) {
        LogPrint(BCLog::WALLET, "chainActive is up-to-date, skipping SPV processing block %d\n", pindex->nHeight);
        if (lastProcessed != headerChain.Tip()) {
            UpdateLastProcessed(chainActive.Tip());
            requestTip = lastProcessed;
        }
    }

    // if this is the block we're waiting for process it and request new block(s)
    if (pindex->pprev == lastProcessed) {
        LogPrint(BCLog::WALLET, "SPV processing block %d\n", pindex->nHeight);

        // TODO handle mempool effects

        std::vector<CTransactionRef> vtxConflicted; // dummy for now
        wallet.BlockConnected(block, pindex, vtxConflicted);

        UpdateLastProcessed(pindex);

        RequestBlocks();
    }
}

void CSPVScanner::HeaderTipChanged(const CBlockIndex* pTip)
{
    RequestBlocks();
}

void CSPVScanner::UpdateLastProcessed(const CBlockIndex* pindex)
{
    lastProcessed = pindex;
    CWalletDB walletdb(*wallet.dbw);
    walletdb.WriteLastSPVBlockProcessed(headerChain.GetLocatorPoW2(lastProcessed));
}
