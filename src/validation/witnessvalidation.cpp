// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2017-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "validation/validation.h"
#include "validation/witnessvalidation.h"
#include <consensus/validation.h>
#include <witnessutil.h>
#include "timedata.h" // GetAdjustedTime()

#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

#include <boost/foreach.hpp> // Zreverse_foreach
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/thread.hpp>

#include "alert.h"



CWitViewDB *ppow2witdbview = NULL;
std::shared_ptr<CCoinsViewCache> ppow2witTip = NULL;

//fixme: (PHASE5) Can remove this.
int GetPoW2WitnessCoinbaseIndex(const CBlock& block)
{
    int commitpos = -1;
    if (!block.vtx.empty()) {
        for (size_t o = 0; o < block.vtx[0]->vout.size(); o++) {
            if (block.vtx[0]->vout[o].GetType() <= CTxOutType::ScriptLegacyOutput)
            {
                if (block.vtx[0]->vout[o].output.scriptPubKey.size() == 143 && block.vtx[0]->vout[o].output.scriptPubKey[0] == OP_RETURN && block.vtx[0]->vout[o].output.scriptPubKey[1] == 0x50 && block.vtx[0]->vout[o].output.scriptPubKey[2] == 0x6f && block.vtx[0]->vout[o].output.scriptPubKey[3] == 0x57 && block.vtx[0]->vout[o].output.scriptPubKey[4] == 0xc2 && block.vtx[0]->vout[o].output.scriptPubKey[5] == 0xb2) {
                    commitpos = o;
                }
            }
        }
    }
    return commitpos;
}

std::vector<CBlockIndex*> GetTopLevelPoWOrphans(const int64_t nHeight, const uint256& prevHash)
{
    LOCK(cs_main);
    std::vector<CBlockIndex*> vRet;
    for (const auto candidateIter : setBlockIndexCandidates)
    {
        if (candidateIter->nVersionPoW2Witness == 0)
        {
            if (candidateIter->nHeight >= nHeight)
            {
                vRet.push_back(candidateIter);
            }
        }
    }
    return vRet;
}

std::vector<CBlockIndex*> GetTopLevelWitnessOrphans(const int64_t nHeight)
{
    LOCK(cs_main);
    std::vector<CBlockIndex*> vRet;
    for (const auto candidateIter : setBlockIndexCandidates)
    {
        if (candidateIter->nVersionPoW2Witness != 0)
        {
            if (candidateIter->nHeight >= nHeight)
            {
                vRet.push_back(candidateIter);
            }
        }
    }
    return vRet;
}

CBlockIndex* GetWitnessOrphanForBlock(const int64_t nHeight, const uint256& prevHash, const uint256& powHash)
{
    LOCK(cs_main);
    for (const auto candidateIter : setBlockIndexCandidates)
    {
        if (candidateIter->nVersionPoW2Witness != 0)
        {
            if (candidateIter->nHeight == nHeight && candidateIter->pprev && *candidateIter->pprev->phashBlock == prevHash)
            {
                if (candidateIter->GetBlockHashLegacy() == powHash)
                {
                    return candidateIter;
                }
            }
        }
    }
    return NULL;
}

static bool ForceActivateChainStep(CValidationState& state, CChain& currentChain, const CChainParams& chainparams, CBlockIndex* pindexMostWork, const std::shared_ptr<const CBlock>& pblock, bool& fInvalidFound, CCoinsViewCache& coinView)
{
    AssertLockHeld(cs_main); // Required for ReadBlockFromDisk.
    const CBlockIndex *pindexFork = currentChain.FindFork(pindexMostWork);

    if (!pindexFork)
    {
        while (currentChain.Tip() && currentChain.Tip()->nHeight >= pindexMostWork->nHeight - 1)
        {
            CBlockIndex* pindexNew = currentChain.Tip()->pprev;
            std::shared_ptr<CBlock> pblock = std::make_shared<CBlock>();
            CBlock& block = *pblock;
            if (!ReadBlockFromDisk(block, currentChain.Tip(), chainparams))
                return false;
            if (DisconnectBlock(block, currentChain.Tip(), coinView) != DISCONNECT_OK)
                return false;
            currentChain.SetTip(pindexNew);
        }
        pindexFork = currentChain.FindFork(pindexMostWork);
    }

    // Disconnect active blocks which are no longer in the best chain.
    while (currentChain.Tip() && currentChain.Tip() != pindexFork) {
        CBlockIndex* pindexNew = currentChain.Tip()->pprev;
        std::shared_ptr<CBlock> pblock = std::make_shared<CBlock>();
        CBlock& block = *pblock;
        if (!ReadBlockFromDisk(block, currentChain.Tip(), chainparams))
            return false;
        if (DisconnectBlock(block, currentChain.Tip(), coinView) != DISCONNECT_OK)
            return false;
        currentChain.SetTip(pindexNew);
    }

    // Build list of new blocks to connect.
    std::vector<CBlockIndex*> vpindexToConnect;
    bool fContinue = true;
    int nHeight = pindexFork ? pindexFork->nHeight : -1;
    while (fContinue && nHeight != pindexMostWork->nHeight) {
        // Don't iterate the entire list of potential improvements toward the best tip, as we likely only need
        // a few blocks along the way.
        int nTargetHeight = std::min(nHeight + 32, pindexMostWork->nHeight);
        vpindexToConnect.clear();
        vpindexToConnect.reserve(nTargetHeight - nHeight);
        CBlockIndex *pindexIter = pindexMostWork->GetAncestor(nTargetHeight);
        while (pindexIter && pindexIter->nHeight != nHeight) {
            vpindexToConnect.push_back(pindexIter);
            pindexIter = pindexIter->pprev;
        }
        nHeight = nTargetHeight;

        // Connect new blocks.
        BOOST_REVERSE_FOREACH(CBlockIndex *pindexConnect, vpindexToConnect) {
            std::shared_ptr<CBlock> pblockConnect = nullptr;
            if (pindexConnect != pindexMostWork || !pblock)
            {
                pblockConnect = std::make_shared<CBlock>();
                CBlock& block = *pblockConnect;
                if (!ReadBlockFromDisk(block, pindexConnect, chainparams))
                    return false;
            }
            bool rv = ConnectBlock(currentChain, pblockConnect?*pblockConnect:*pblock, state, pindexConnect, coinView, chainparams);
            if (!rv)
                return false;
            currentChain.SetTip(pindexConnect);
        }
    }

    return true;
}

// pblock is either NULL or a pointer to a CBlock corresponding to pActiveIndex, to bypass loading it again from disk.
bool ForceActivateChain(CBlockIndex* pActivateIndex, std::shared_ptr<const CBlock> pblock, CValidationState& state, const CChainParams& chainparams, CChain& currentChain, CCoinsViewCache& coinView)
{
    DO_BENCHMARK("WIT: ForceActivateChain", BCLog::BENCH|BCLog::WITNESS);

    CBlockIndex* pindexNewTip = nullptr;
    do {
        {
            LOCK(cs_main);

            // Whether we have anything to do at all.
            if (pActivateIndex == NULL || pActivateIndex == currentChain.Tip())
                return true;

            bool fInvalidFound = false;
            std::shared_ptr<const CBlock> nullBlockPtr;
            if (!ForceActivateChainStep(state, currentChain, chainparams, pActivateIndex, pblock, fInvalidFound, coinView))
                return false;

            if (fInvalidFound) {
                return false;
            }
            pindexNewTip = currentChain.Tip();
        }
        // When we reach this point, we switched to a new tip (stored in pindexNewTip).
    } while (pindexNewTip != pActivateIndex);

    return true;
}

bool ForceActivateChainWithBlockAsTip(CBlockIndex* pActivateIndex, std::shared_ptr<const CBlock> pblock, CValidationState& state, const CChainParams& chainparams, CChain& currentChain, CCoinsViewCache& coinView, CBlockIndex* pnewblockastip)
{
    if(!ForceActivateChain(pActivateIndex, pblock, state, chainparams, currentChain, coinView))
        return false;
    return ForceActivateChain(pnewblockastip, nullptr, state, chainparams, currentChain, coinView);
}

uint64_t expectedWitnessBlockPeriod(uint64_t nWeight, uint64_t networkTotalWeight)
{
    if (nWeight == 0 || networkTotalWeight == 0)
        return 0;

    if (nWeight > networkTotalWeight/100)
        nWeight = networkTotalWeight/100;

    static const arith_uint256 base = arith_uint256(100000000) * arith_uint256(100000000) * arith_uint256(100000000);
    #define BASE(x) (arith_uint256(x)*base)
    #define AI(x) arith_uint256(x)
    return 100 + std::max(( ((BASE(1)/((BASE(nWeight)/AI(networkTotalWeight))))).GetLow64() * 10 ), (uint64_t)1000);
    #undef AI
    #undef BASE
}

uint64_t estimatedWitnessBlockPeriod(uint64_t nWeight, uint64_t networkTotalWeight)
{
    DO_BENCHMARK("WIT: estimatedWitnessBlockPeriod", BCLog::BENCH|BCLog::WITNESS);

    if (nWeight == 0 || networkTotalWeight == 0)
        return 0;

    if (nWeight > networkTotalWeight/100)
        nWeight = networkTotalWeight/100;

    static const arith_uint256 base = arith_uint256(100000000) * arith_uint256(100000000) * arith_uint256(100000000);
    #define BASE(x) (arith_uint256(x)*base)
    #define AI(x) arith_uint256(x)
    return 100 + ((BASE(1)/((BASE(nWeight)/AI(networkTotalWeight))))).GetLow64();
    #undef AI
    #undef BASE
}


bool getAllUnspentWitnessCoins(CChain& chain, const CChainParams& chainParams, const CBlockIndex* pPreviousIndexChain_, std::map<COutPoint, Coin>& allWitnessCoins, CBlock* newBlock, CCoinsViewCache* viewOverride)
{
    DO_BENCHMARK("WIT: getAllUnspentWitnessCoins", BCLog::BENCH|BCLog::WITNESS);

    #ifdef ENABLE_WALLET
    LOCK2(cs_main, pactiveWallet?&pactiveWallet->cs_wallet:NULL);
    #else
    LOCK(cs_main);
    #endif

    assert(pPreviousIndexChain_);

    allWitnessCoins.clear();
    //fixme: (PHASE5) Add more error handling to this function.
    // Sort out pre-conditions.
    // We have to make sure that we are using a view and chain that includes the PoW block we are witnessing and all of its transactions as the tip.
    // It won't necessarily be part of the chain yet; if we are in the process of witnessing; or if the block is an older one on a fork; because only blocks that have already been witnessed can be part of the chain.
    // So we have to temporarily force disconnect/reconnect of blocks as necessary to make a temporary working chain that suits the properties we want.
    // NB!!! - It is important that we don't flush either of these before destructing, we want to throw the result away.
    CCoinsViewCache viewNew(viewOverride?viewOverride:pcoinsTip);

    if ((uint64_t)pPreviousIndexChain_->nHeight < Params().GetConsensus().pow2Phase2FirstBlockHeight)
        return true;

    // We work on a clone of the chain to prevent modifying the actual chain.
    CBlockIndex* pPreviousIndexChain = nullptr;
    CCloneChain tempChain(chain, GetPow2ValidationCloneHeight(chain, pPreviousIndexChain_, 2), pPreviousIndexChain_, pPreviousIndexChain);
    CValidationState state;
    assert(pPreviousIndexChain);

    // Force the tip of the chain to the block that comes before the block we are examining.
    // For phase 3 this must be a PoW block - from phase 4 it should be a witness block 
    if (pPreviousIndexChain->nHeight == 0)
    {
        ForceActivateChain(pPreviousIndexChain, nullptr, state, chainParams, tempChain, viewNew);
    }
    else
    {
        if (pPreviousIndexChain->nVersionPoW2Witness==0 || IsPow2Phase4Active(pPreviousIndexChain->pprev))
        {
            ForceActivateChain(pPreviousIndexChain, nullptr, state, chainParams, tempChain, viewNew);
        }
        else
        {
            CBlockIndex* pPreviousIndexChainPoW = new CBlockIndex(*GetPoWBlockForPoSBlock(pPreviousIndexChain));
            assert(pPreviousIndexChainPoW);
            pPreviousIndexChainPoW->pprev = pPreviousIndexChain->pprev;
            ForceActivateChainWithBlockAsTip(pPreviousIndexChain->pprev, nullptr, state, chainParams, tempChain, viewNew, pPreviousIndexChainPoW);
            pPreviousIndexChain = tempChain.Tip();
        }
    }

    // If we have been passed a new tip block (not yet part of the chain) then add it to the chain now.
    if (newBlock)
    {
        // Strip any witness information from the block we have been given we want a non-witness block as the tip in order to calculate the witness for it.
        if (newBlock->nVersionPoW2Witness != 0)
        {
            for (unsigned int i = 1; i < newBlock->vtx.size(); i++)
            {
                if (newBlock->vtx[i]->IsCoinBase() && newBlock->vtx[i]->IsPoW2WitnessCoinBase())
                {
                    while (newBlock->vtx.size() > i)
                    {
                        newBlock->vtx.pop_back();
                    }
                    break;
                }
            }
            newBlock->nVersionPoW2Witness = 0;
            newBlock->nTimePoW2Witness = 0;
            newBlock->hashMerkleRootPoW2Witness = uint256();
            newBlock->witnessHeaderPoW2Sig.clear();
        }

        // Place the block in question at the tip of the chain.
        CBlockIndex indexDummy(*newBlock);
        indexDummy.pprev = pPreviousIndexChain;
        indexDummy.nHeight = pPreviousIndexChain->nHeight + 1;
        if (!ConnectBlock(tempChain, *newBlock, state, &indexDummy, viewNew, chainParams, true, false))
        {
            //fixme: (PHASE5) If we are inside a GetWitness call ban the peer that sent us this?
            return false;
        }
    }

    /** Gather a list of all unspent witness outputs.
        NB!!! There are multiple layers of cache at play here, with insertions/deletions possibly having taken place at each layer.
        Therefore the order of operations is crucial, we must first iterate the lowest layer, then the second lowest and finally the highest layer.
        For each iteration we should remove items from allWitnessCoins if they have been deleted in the higher layer as the higher layer overrides the lower layer.
        GetAllCoins takes care of all of this automatically.
    **/
    viewNew.pChainedWitView->GetAllCoins(allWitnessCoins);

    return true;
}


//fixme: (PHASE5) Improve error handling.
//fixme: (PHASE5) Handle nodes with excessive pruning. //pblocktree->ReadFlag("prunedblockfiles", fHavePruned);
bool GetWitnessHelper(uint256 blockHash, CGetWitnessInfo& witnessInfo, uint64_t nBlockHeight)
{
    DO_BENCHMARK("WIT: GetWitnessHelper", BCLog::BENCH|BCLog::WITNESS);

    #ifdef ENABLE_WALLET
    LOCK2(cs_main, pactiveWallet?&pactiveWallet->cs_wallet:nullptr);
    #else
    LOCK(cs_main);
    #endif

    /** Generate the pool of potential witnesses for the given block index **/
    /** Addresses younger than nMinAge blocks are discarded **/
    uint64_t nMinAge = gMinimumParticipationAge;
    while (true)
    {
        witnessInfo.witnessSelectionPoolFiltered.clear();
        witnessInfo.witnessSelectionPoolFiltered = witnessInfo.witnessSelectionPoolUnfiltered;

        /** Eliminate addresses that have witnessed within the last `gMinimumParticipationAge` blocks **/
        witnessInfo.witnessSelectionPoolFiltered.erase(std::remove_if(witnessInfo.witnessSelectionPoolFiltered.begin(), witnessInfo.witnessSelectionPoolFiltered.end(), [&](RouletteItem& x){ return (x.nAge <= nMinAge); }), witnessInfo.witnessSelectionPoolFiltered.end());

        /** Eliminate addresses that have not witnessed within the expected period of time that they should have **/
        witnessInfo.witnessSelectionPoolFiltered.erase(std::remove_if(witnessInfo.witnessSelectionPoolFiltered.begin(), witnessInfo.witnessSelectionPoolFiltered.end(), [&](RouletteItem& x){ return witnessHasExpired(x.nAge, x.nWeight, witnessInfo.nTotalWeightRaw); }), witnessInfo.witnessSelectionPoolFiltered.end());

        /** Eliminate addresses that are within 100 blocks from lock period expiring, or whose lock period has expired. **/
        witnessInfo.witnessSelectionPoolFiltered.erase(std::remove_if(witnessInfo.witnessSelectionPoolFiltered.begin(), witnessInfo.witnessSelectionPoolFiltered.end(), [&](RouletteItem& x){ CTxOutPoW2Witness details; GetPow2WitnessOutput(x.coin.out, details); return (GetPoW2RemainingLockLengthInBlocks(details.lockUntilBlock, nBlockHeight) <= nMinAge); }), witnessInfo.witnessSelectionPoolFiltered.end());

        // We must have at least 100 accounts to keep odds of being selected down below 1% at all times.
        if (witnessInfo.witnessSelectionPoolFiltered.size() < 100)
        {
            if(!IsArgSet("-testnet") && nBlockHeight > 880000)
                CAlert::Notify("Warning network is experiencing low levels of witnessing participants!", true, true);


            // NB!! This part of the code should (ideally) never actually be used, it exists only for instances where there are a shortage of witnesses paticipating on the network.
            if (nMinAge == 0 || (nMinAge <= 10 && witnessInfo.witnessSelectionPoolFiltered.size() > 5))
            {
                break;
            }
            else
            {
                // Try again to reach 100 candidates with a smaller min age.
                nMinAge -= 5;
            }
        }
        else
        {
            break;
        }
    }

    if (witnessInfo.witnessSelectionPoolFiltered.size() == 0)
    {
        return error("Unable to determine any witnesses for block.");
    }

    /** Ensure the pool is sorted deterministically **/
    std::sort(witnessInfo.witnessSelectionPoolFiltered.begin(), witnessInfo.witnessSelectionPoolFiltered.end());

    /** Calculate total eligible weight **/
    witnessInfo.nTotalWeightEligibleRaw = 0;
    for (auto& item : witnessInfo.witnessSelectionPoolFiltered)
    {
        witnessInfo.nTotalWeightEligibleRaw += item.nWeight;
    }

    uint64_t genesisWeight=0;
    if (Params().numGenesisWitnesses > 0)
    {
        genesisWeight = std::max(witnessInfo.nTotalWeightEligibleRaw / Params().genesisWitnessWeightDivisor, (uint64_t)1000);
        witnessInfo.nTotalWeightEligibleRaw += Params().numGenesisWitnesses*genesisWeight;
    }
    
    /** Reduce larger weightings to a maximum weighting of 1% of network weight. **/
    /** NB!! this actually will end up a little bit more than 1% as the overall network weight will also be reduced as a result. **/
    /** This is however unimportant as 1% is in and of itself also somewhat arbitrary, simpler code is favoured here over exactness. **/
    /** So we delibritely make no attempt to compensate for this. **/
    witnessInfo.nMaxIndividualWeight = witnessInfo.nTotalWeightEligibleRaw / 100;
    witnessInfo.nTotalWeightEligibleAdjusted = 0;
    for (auto& item : witnessInfo.witnessSelectionPoolFiltered)
    {
        if (item.nWeight == 0)
            item.nWeight = genesisWeight;
        if (item.nWeight > witnessInfo.nMaxIndividualWeight)
            item.nWeight = witnessInfo.nMaxIndividualWeight;
        witnessInfo.nTotalWeightEligibleAdjusted += item.nWeight;
        item.nCumulativeWeight = witnessInfo.nTotalWeightEligibleAdjusted;
    }

    /** sha256 as random roulette spin/seed - NB! We delibritely use sha256 and -not- the normal PoW hash here as the normal PoW hash is biased towards certain number ranges by -design- (block target) so is not a good RNG... **/
    arith_uint256 rouletteSelectionSeed = UintToArith256(blockHash);

    //fixme: (PHASE5) Update whitepaper then delete this code.
    /** ensure random seed exceeds one full spin of the wheel to prevent any possible bias towards low numbers **/
    //while (rouletteSelectionSeed < witnessInfo.nTotalWeightEligibleAdjusted)
    //{
        //rouletteSelectionSeed = rouletteSelectionSeed * 2;
    //}

    /** Reduce selection number to fit within possible range of values **/
    if (rouletteSelectionSeed > arith_uint256(witnessInfo.nTotalWeightEligibleAdjusted))
    {
        // 'BigNum' Modulo operator via mathematical identity:  a % b = a - (b * int(a/b))
        rouletteSelectionSeed = rouletteSelectionSeed - (arith_uint256(witnessInfo.nTotalWeightEligibleAdjusted) * arith_uint256(rouletteSelectionSeed/arith_uint256(witnessInfo.nTotalWeightEligibleAdjusted)));
    }

    /** Perform selection **/
    auto selectedWitness = std::lower_bound(witnessInfo.witnessSelectionPoolFiltered.begin(), witnessInfo.witnessSelectionPoolFiltered.end(), rouletteSelectionSeed.GetLow64());
    witnessInfo.selectedWitnessTransaction = selectedWitness->coin.out;
    witnessInfo.selectedWitnessBlockHeight = selectedWitness->coin.nHeight;
    witnessInfo.selectedWitnessOutpoint = selectedWitness->outpoint;

    return true;
}

bool GetWitnessInfo(CChain& chain, const CChainParams& chainParams, CCoinsViewCache* viewOverride, CBlockIndex* pPreviousIndexChain, CBlock block, CGetWitnessInfo& witnessInfo, uint64_t nBlockHeight)
{
    DO_BENCHMARK("WIT: GetWitnessInfo", BCLog::BENCH|BCLog::WITNESS);

    #ifdef DISABLE_WALLET
    LOCK2(cs_main, pactiveWallet?&pactiveWallet->cs_wallet:nullptr);
    #else
    LOCK(cs_main);
    #endif

    // Fetch all unspent witness outputs for the chain in which -block- acts as the tip.
    if (!getAllUnspentWitnessCoins(chain, chainParams, pPreviousIndexChain, witnessInfo.allWitnessCoins, &block, viewOverride))
        return false;

    // Gather all witnesses that exceed minimum weight and count the total witness weight.
    for (auto coinIter : witnessInfo.allWitnessCoins)
    {
        //fixme: (PHASE5) Unit tests
        uint64_t nAge = nBlockHeight - coinIter.second.nHeight;
        COutPoint outPoint = coinIter.first;
        assert(outPoint.isHash);
        Coin coin = coinIter.second;
        if (coin.out.nValue >= (gMinimumWitnessAmount*COIN))
        {
            uint64_t nUnused1, nUnused2;
            int64_t nWeight = GetPoW2RawWeightForAmount(coin.out.nValue, GetPoW2LockLengthInBlocksFromOutput(coin.out, coin.nHeight, nUnused1, nUnused2));
            if (nWeight < gMinimumWitnessWeight)
                continue;
            witnessInfo.witnessSelectionPoolUnfiltered.push_back(RouletteItem(outPoint, coin, nWeight, nAge));
            witnessInfo.nTotalWeightRaw += nWeight;
        }
        else if (coin.out.output.witnessDetails.lockFromBlock == 1)
        {
            int64_t nWeight = 0;
            witnessInfo.witnessSelectionPoolUnfiltered.push_back(RouletteItem(outPoint, coin, nWeight, nAge));                     
        }
    }

    // On testnet v1 we have some abandoned phase 3 accounts that will never be upgraded, we need to add their weight here
    if (chainParams.IsOfficialTestnetV1())
    {
        witnessInfo.nTotalWeightRaw += 405911+5827397+33566+524931+242054+44000+33825+156000+33825+2400000+21000+109093+496438+8000000+21000+10481+1499998+405911+156000+4799994+649457+20999+800000+1499998+2399997+16454794+15219+21000+397808+2828758+299999;
    }
    
    return true;
}

bool GetWitness(CChain& chain, const CChainParams& chainParams, CCoinsViewCache* viewOverride, CBlockIndex* pPreviousIndexChain, CBlock block, CGetWitnessInfo& witnessInfo)
{
    DO_BENCHMARK("WIT: GetWitness", BCLog::BENCH|BCLog::WITNESS);

    #ifdef ENABLE_WALLET
    LOCK2(cs_main, pactiveWallet?&pactiveWallet->cs_wallet:nullptr);
    #else
    LOCK(cs_main);
    #endif

    // Fetch all the chain info (for specific block) we will need to calculate the witness.
    uint64_t nBlockHeight = pPreviousIndexChain->nHeight + 1;
    if (!GetWitnessInfo(chain, chainParams, viewOverride, pPreviousIndexChain, block, witnessInfo, nBlockHeight))
        return false;

    return GetWitnessHelper(block.GetHashLegacy(), witnessInfo, nBlockHeight);
}

// Ideally this should have been some hybrid of witInfo.nTotalWeight / witInfo.nReducedTotalWeight - as both independantly aren't perfect.
// Total weight is prone to be too high if there are lots of large >1% witnesses, nReducedTotalWeight is prone to be too low if there is one large witness who has recently witnessed.
// However on a large network with lots of participants this should not matter - and technical constraints make the total the best compromise
// As we need to call this from within the witness algorithm from before nReducedTotalWeight is even known. 
bool witnessHasExpired(uint64_t nWitnessAge, uint64_t nWitnessWeight, uint64_t nNetworkTotalWitnessWeight)
{
    if (nWitnessWeight == 0)
        return false;

    uint64_t nExpectedWitnessPeriod = expectedWitnessBlockPeriod(nWitnessWeight, nNetworkTotalWitnessWeight);
    return ( nWitnessAge > gMaximumParticipationAge ) || ( nWitnessAge > nExpectedWitnessPeriod );
}

