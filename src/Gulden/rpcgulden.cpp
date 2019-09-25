// Copyright (c) 2015-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "rpcgulden.h"
#include "generation/generation.h"
#include "generation/miner.h"
#include "generation/witness.h"
#include <rpc/server.h>
#include "validation/validation.h"
#include "validation/witnessvalidation.h"
#include <consensus/consensus.h>
#include <boost/assign/list_of.hpp>

#include "auto_checkpoints.h"

#ifdef ENABLE_WALLET
#include <wallet/rpcwallet.h>
#include "wallet/wallet.h"
#include "wallet/coincontrol.h"
#include "wallet/wallet.h"
#include "wallet/witness_operations.h"
#endif

#include <univalue.h>

#include <Gulden/util.h>
#include <numeric>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/algorithm/string/predicate.hpp> // for starts_with() and ends_with()

#include "txdb.h"
#include "coins.h"
#include "blockfilter.h"
#include "primitives/transaction.h"

#include <Gulden/util.h>
#include "utilmoneystr.h"

#include <consensus/validation.h>
#include <consensus/consensus.h>
#include "net.h"


#ifdef ENABLE_WALLET

static UniValue gethashps(const JSONRPCRequest& request)
{
    if (request.fHelp)
        throw std::runtime_error(
            "gethashps\n"
            "\nReturns the estimated hashes per second that this computer is mining at.\n");

    if (dHashesPerSec > 1000000)
        return strprintf("%lf Mh/s (best %lf Mh/s)", dHashesPerSec/1000000, dBestHashesPerSec/1000000);
    else if (dHashesPerSec > 1000)
        return strprintf("%lf Kh/s (best %ls Kh/s)", dHashesPerSec/1000, dBestHashesPerSec/1000);
    else
        return strprintf("%lf h/s (best %lf h/s)", dHashesPerSec, dBestHashesPerSec);
}

static UniValue sethashlimit(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "sethashlimit  ( limit )\n"
            "\nSet the maximum number of hashes to calculate per second when mining.\n"
            "\nThis mainly exists for testing purposes but can also be used to limit CPU usage a little.\n"
            "\nArguments:\n"
            "1. limit     (numeric) The number of hashes to allow per second, or -1 to remove limit.\n"
            "\nExamples:\n"
            + HelpExampleCli("sethashlimit 500000", "")
            + HelpExampleRpc("sethashlimit 500000", ""));

    RPCTypeCheck(request.params, boost::assign::list_of(UniValue::VNUM));

    nHashThrottle = request.params[0].get_int();

    LogPrintf("<DELTA> hash throttle %ld\n", nHashThrottle);

    return strprintf("Throttling hash: %d", nHashThrottle);
}

static UniValue getwitnessinfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 3)
        throw std::runtime_error(
            "getwitnessinfo \"block_specifier\" verbose mine_only\n"
            "\nReturns witness related network info for a given block."
            "\nWhen verbose is enabled returns additional statistics.\n"
            "\nArguments:\n"
            "1. \"block_specifier\"       (string, optional, default=tip) The block_specifier for which to display witness information, if empty or 'tip' the tip of the current chain is used.\n"
            "\nSpecifier can be the hash of the block; an absolute height in the blockchain or a tip~# specifier to iterate backwards from tip; for which to return witness details\n"
            "2. verbose                  (boolean, optional, default=false) Display additional verbose information.\n"
            "3. mine_only                (boolean, optional, default=false) In verbose display only show account info for accounts belonging to this wallet.\n"
            "\nResult:\n"
            "[{\n"
            "     \"pow2_phase\": n                                  (number) The number of the currently active pow2_phase.\n"
            "     \"number_of_witnesses_raw\": n                     (number) The total number of funded witness addresses in existence on the network.\n"
            "     \"number_of_witnesses_total\": n                   (number) The total number of funded witness addresses in existence on the network which pass basic restrictions to witness like minimum weight.\n"
            "     \"number_of_witnesses_eligible\": n                (number) The total number of witness addresses on the network which were considered eligible candidates to witness for this block.\n"
            "     \"total_witness_weight_raw\": n                    (number) The total weight of all witness addresses that were part of \"number_of_witnesses_total\".\n"
            "     \"total_witness_weight_eligible_raw\": n,          (number) The total weight of all witness addresses that were part of \"number_of_witnesses_eligible\".\n"
            "     \"total_witness_weight_eligible_adjusted\": n,     (number) The adjusted weight (after applying maximum weight restrictions) of all witness addresses that were part of \"number_of_witnesses_eligible\".\n"
            "     \"selected_witness_address\": address              (string) The address of the witness that has been selected for the current chain tip.\n"
            "     \"witness_statistics\": {\n"
            "         \"weight\": {                                  Weight statistics based on all witness addresses\n"
            "             \"largest\": n                             (number) The largest single address weight on the network.\n"
            "             \"smallest\": n                            (number) The smallest single address weight on the network.\n"
            "             \"mean\": n                                (number) The mean weight of all witness addresses.\n"
            "             \"median\": n                              (number) The median weight of all witness addresses.\n"
            "         }\n"
            "         \"amount\": {                                  Amount statistics based on all witness addresses\n"
            "             \"largest\": n                             (number) The largest single address amount on the network.\n"
            "             \"smallest\": n                            (number) The smallest single address amount on the network.\n"
            "             \"mean\": n                                (number) The mean amount of all witness addresses.\n"
            "             \"median\": n                              (number) The median amount of all witness addresses.\n"
            "         }\n"
            "         \"lock_period\": {                             Lock period statistics based on all witness addresses\n"
            "             \"largest\": n                             (number) The largest single address lock_period on the network.\n"
            "             \"smallest\": n                            (number) The smallest single address lock_period on the network.\n"
            "             \"mean\": n                                (number) The mean lock_period of all witness addresses.\n"
            "             \"median\": n                              (number) The median lock_period of all witness addresses.\n"
            "         }\n"
            "         \"age\": {                                     Age statistics based on all witness addresses (age is how long an address has existed since it last performed an operation of some kind)\n"
            "             \"largest\": n                             (number) The oldest address on the network.\n"
            "             \"smallest\": n                            (number) The more recent address on the network.\n"
            "             \"mean\": n                                (number) The mean age of all witness addresses.\n"
            "             \"median\": n                              (number) The median age of all witness addresses.\n"
            "         }\n"
            "     }\n"
            "     \"witness_address_list\": [                        List of all witness addresses on the network, with address specific information\n"
            "         {\n"
            "             \"type\": address_type                     (string) The type of address output used to create the address. Either SCRIPT or POW2WITNESS depending on whether SegSig was activated at the time of creation or not.\n"
            "             \"address\": address                       (string) The address of the witness that has been selected for the current chain tip.\n"
            "             \"age\": n                                 (number) The age of the address (how long since it was last active in any way)\n"
            "             \"amount\": n                              (number) The amount that is locked in the address.\n"
            "             \"raw_weight\": n                          (number) The raw weight of the address before any adjustments.\n"
            "             \"adjusted_weight\": n                     (number) The weight after 1% limit is applied\n"
            "             \"adjusted_weight_final\": n               (number) The weight considered by the witness algorithm after all adjustments are applied.\n"
            "             \"expected_witness_period\": n             (number) The period that the network will allow this address to go without witnessing before it expires.\n"
            "             \"estimated_witness_period\": n            (number) The average period in which this address should earn a reward over time\n"
            "             \"last_active_block\": n                   (number) The last block in which this address was active.\n"
            "             \"lock_from_block\": n                     (number) The block where this address was originally locked.\n"
            "             \"lock_until_block\": n                    (number) The block that this address will remain locked until.\n"
            "             \"lock_period\": n                         (number) The complete length in time that this address will be locked for\n"
            "             \"lock_period_expired\": n                 (boolean)true if the lock has expired (funds can be withdrawed)\n"
            "             \"eligible_to_witness\": n                 (number) true if the address is eligible to witness.\n"
            "             \"expired_from_inactivity\": n             (number) true if the network has expired (kicked off) this address due to it failing to witness in the expected period\n"
            //"             \"fail_count\": n                          (number) Internal accounting for how many times this address has been renewed; Note it increases in a non-linear fashion but decreases by 1 for every valid witnessing operation.\n"
            //"             \"action_nonce\": n                        (number) Internal count of how many actions this address has been involved in since creation; Used to ensure address transaction uniqueness across operations.\n"
            "             \"ismine_accountname\": n                  (string) If the address belongs to an account in this wallet, the name of the account.\n"
            "         }\n"
            "         ...\n"
            "     ]\n"
            "}]\n"
            "\nExamples:\n"
            "\nBasic witness info for current chain tip\n"
            + HelpExampleCli("getwitnessinfo tip false", "")
            + "\nExtended witness info for the block two blocks before tip\n"
            + HelpExampleCli("getwitnessinfo tip~2 true", "")
            + "\nExtended witness info for block 400000\n"
            + HelpExampleCli("getwitnessinfo 400000 true", "")
            + "\nExtended witness info for block with hash 8383d8e9999ade8ad0c9f84e7816afec3b9e4855341f678bb0fdc3af46ee6f31\n"
            + HelpExampleCli("getwitnessinfo \"8383d8e9999ade8ad0c9f84e7816afec3b9e4855341f678bb0fdc3af46ee6f31\" true", ""));

    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;
    #else
    LOCK(cs_main);
    #endif

    int64_t nTotalWeightAll = 0;
    int64_t nNumWitnessAddressesAll = 0;
    int64_t nPow2Phase = 1;
    std::string sWitnessAddress;
    UniValue jsonAllWitnessAddresses(UniValue::VARR);
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::median(boost::accumulators::with_p_square_quantile), boost::accumulators::tag::mean, boost::accumulators::tag::min, boost::accumulators::tag::max> > witnessWeightStats;
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::median(boost::accumulators::with_p_square_quantile), boost::accumulators::tag::mean, boost::accumulators::tag::min, boost::accumulators::tag::max> > witnessAmountStats;
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::median(boost::accumulators::with_p_square_quantile), boost::accumulators::tag::mean, boost::accumulators::tag::min, boost::accumulators::tag::max> > lockPeriodWeightStats;
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::median(boost::accumulators::with_p_square_quantile), boost::accumulators::tag::mean, boost::accumulators::tag::min, boost::accumulators::tag::max> > ageStats;

    CBlockIndex* pTipIndex = nullptr;
    bool fVerbose = false;
    bool showMineOnly = false;
    if (request.params.size() > 0)
    {
        std::string sTipHash = request.params[0].get_str();
        int32_t nTipHeight;
        if (ParseInt32(sTipHash, &nTipHeight))
        {
            pTipIndex = chainActive[nTipHeight];
            if (!pTipIndex)
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found.");
        }
        else
        {
            if (sTipHash == "tip" || sTipHash.empty())
            {
                pTipIndex = chainActive.Tip();
                if (!pTipIndex)
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Chain has no tip.");
            }
            else if(boost::starts_with(sTipHash, "tip~"))
            {
                int nReverseHeight;
                if (!ParseInt32(sTipHash.substr(4,std::string::npos), &nReverseHeight))
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid block specifier.");
                pTipIndex = chainActive.Tip();
                if (!pTipIndex)
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Chain has no tip.");
                while(pTipIndex && nReverseHeight>0)
                {
                    pTipIndex = pTipIndex->pprev;
                    --nReverseHeight;
                }
                if (!pTipIndex)
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid block specifier, chain does not go back that far.");
            }
            else
            {
                uint256 hash(uint256S(sTipHash));
                if (mapBlockIndex.count(hash) == 0)
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found.");
                pTipIndex = mapBlockIndex[hash];
            }
        }
    }
    else
    {
        pTipIndex = chainActive.Tip();
    }

    if (!pTipIndex || pTipIndex->nHeight < GetPhase2ActivationHeight())
        return NullUniValue;

    if (request.params.size() >= 2)
        fVerbose = request.params[1].get_bool();

    if (request.params.size() > 2)
        showMineOnly = request.params[2].get_bool();

    CBlockIndex* pTipIndex_ = nullptr;
    //fixme: (2.0.x) - Fix this to only do a shallow clone of whats needed (need to fix recursive cloning mess first)
    CCloneChain tempChain(chainActive, GetPow2ValidationCloneHeight(chainActive, pTipIndex, 10), pTipIndex, pTipIndex_);

    if (!pTipIndex_)
        throw std::runtime_error("Could not locate a valid PoW² chain that contains this block as tip.");
    CCoinsViewCache viewNew(pcoinsTip);
    CValidationState state;
    if (!ForceActivateChain(pTipIndex_, nullptr, state, Params(), tempChain, viewNew))
        throw std::runtime_error("Could not locate a valid PoW² chain that contains this block as tip.");
    if (!state.IsValid())
        throw JSONRPCError(RPC_DATABASE_ERROR, state.GetRejectReason());

    if (IsPow2Phase5Active(pTipIndex_, Params(), tempChain, &viewNew))
        nPow2Phase = 5;
    else if (IsPow2Phase4Active(pTipIndex_, Params(), tempChain, &viewNew))
        nPow2Phase = 4;
    else if (IsPow2Phase3Active(pTipIndex_->nHeight))
        nPow2Phase = 3;
    else if (IsPow2Phase2Active(pTipIndex_, Params(), tempChain, &viewNew))
        nPow2Phase = 2;

    CGetWitnessInfo witInfo;

    if (nPow2Phase >= 2)
    {
        CBlock block;
        {
            LOCK(cs_main);// cs_main lock required for ReadBlockFromDisk
            if (!ReadBlockFromDisk(block, pTipIndex_, Params()))
                throw std::runtime_error("Could not load block to obtain PoW² information.");
        }

        if (!GetWitnessInfo(tempChain, Params(), &viewNew, pTipIndex_->pprev, block, witInfo, pTipIndex_->nHeight))
            throw std::runtime_error("Could not enumerate all PoW² witness information for block.");

        if (!GetPow2NetworkWeight(pTipIndex_, Params(), nNumWitnessAddressesAll, nTotalWeightAll, tempChain, &viewNew))
            throw std::runtime_error("Block does not form part of a valid PoW² chain.");

        if (nPow2Phase >= 3)
        {
            if (!GetWitnessHelper(block.GetHashLegacy(), witInfo, pTipIndex_->nHeight))
                throw std::runtime_error("Could not select a valid PoW² witness for block.");

            CTxDestination selectedWitnessAddress;
            if (!ExtractDestination(witInfo.selectedWitnessTransaction, selectedWitnessAddress))
                throw std::runtime_error("Could not extract PoW² witness for block.");

            sWitnessAddress = CGuldenAddress(selectedWitnessAddress).ToString();
        }
    }

    if (fVerbose)
    {
        for (auto& iter : witInfo.allWitnessCoins)
        {
            bool fEligible = false;
            uint64_t nAdjustedWeight = 0;
            {
                auto poolIter = witInfo.witnessSelectionPoolFiltered.begin();
                while (poolIter != witInfo.witnessSelectionPoolFiltered.end())
                {
                    if (poolIter->outpoint == iter.first)
                    {
                        if (poolIter->coin.out == iter.second.out)
                        {
                            nAdjustedWeight = poolIter->nWeight;
                            fEligible = true;
                            break;
                        }
                    }
                    ++poolIter;
                }
            }
            bool fExpired = false;
            {
                auto poolIter = witInfo.witnessSelectionPoolUnfiltered.begin();
                while (poolIter != witInfo.witnessSelectionPoolUnfiltered.end())
                {
                    if (poolIter->outpoint == iter.first)
                    {
                        if (poolIter->coin.out == iter.second.out)
                        {
                            if (witnessHasExpired(poolIter->nAge, poolIter->nWeight, witInfo.nTotalWeightRaw))
                            {
                                fExpired = true;
                            }
                            break;
                        }
                    }
                    ++poolIter;
                }
            }
    
            CTxDestination address;
            if (!ExtractDestination(iter.second.out, address))
                throw std::runtime_error("Could not extract PoW² witness for block.");
            
            CTxOutPoW2Witness witnessDetails;
            if (!GetPow2WitnessOutput(iter.second.out, witnessDetails))
                throw std::runtime_error("Could not extract PoW² witness details for block.");

            uint64_t nLastActiveBlock = iter.second.nHeight;
            uint64_t nLockFromBlock = 0;
            uint64_t nLockUntilBlock = 0;
            uint64_t nLockPeriodInBlocks = GetPoW2LockLengthInBlocksFromOutput(iter.second.out, iter.second.nHeight, nLockFromBlock, nLockUntilBlock);
            uint64_t nRawWeight = GetPoW2RawWeightForAmount(iter.second.out.nValue, nLockPeriodInBlocks);
            uint64_t nAge = pTipIndex_->nHeight - nLastActiveBlock;
            CAmount nValue = iter.second.out.nValue;

            bool fLockPeriodExpired = (GetPoW2RemainingLockLengthInBlocks(nLockUntilBlock, pTipIndex_->nHeight) == 0);

            #ifdef ENABLE_WALLET
            std::string accountName = accountNameForAddress(*pwallet, address);
            #endif

            UniValue rec(UniValue::VOBJ);
            rec.push_back(Pair("type", iter.second.out.GetTypeAsString()));
            rec.push_back(Pair("address", CGuldenAddress(address).ToString()));
            rec.push_back(Pair("age", nAge));
            rec.push_back(Pair("amount", ValueFromAmount(nValue)));
            rec.push_back(Pair("raw_weight", nRawWeight));
            rec.push_back(Pair("adjusted_weight", std::min(nRawWeight, witInfo.nMaxIndividualWeight)));
            rec.push_back(Pair("adjusted_weight_final", nAdjustedWeight));
            rec.push_back(Pair("expected_witness_period", expectedWitnessBlockPeriod(nRawWeight, witInfo.nTotalWeightRaw)));
            rec.push_back(Pair("estimated_witness_period", estimatedWitnessBlockPeriod(nRawWeight, witInfo.nTotalWeightRaw)));
            rec.push_back(Pair("last_active_block", nLastActiveBlock));
            rec.push_back(Pair("lock_from_block", nLockFromBlock));
            rec.push_back(Pair("lock_until_block", nLockUntilBlock));
            rec.push_back(Pair("lock_period", nLockPeriodInBlocks));
            rec.push_back(Pair("lock_period_expired", fLockPeriodExpired));
            rec.push_back(Pair("eligible_to_witness", fEligible));
            rec.push_back(Pair("expired_from_inactivity", fExpired));
            rec.push_back(Pair("fail_count", witnessDetails.failCount));
            rec.push_back(Pair("action_nonce", witnessDetails.actionNonce));
            #ifdef ENABLE_WALLET
            rec.push_back(Pair("ismine_accountname", accountName));
            #else
            rec.push_back(Pair("ismine_accountname", ""));
            #endif

            witnessWeightStats(nRawWeight);
            lockPeriodWeightStats(nLockPeriodInBlocks);
            witnessAmountStats(nValue);
            ageStats(nAge);

            #ifdef ENABLE_WALLET
            if (showMineOnly && accountName.empty())
                continue;
            #endif

            jsonAllWitnessAddresses.push_back(rec);
        }
    }

    UniValue witnessInfoForBlock(UniValue::VARR);
    UniValue rec(UniValue::VOBJ);
    rec.push_back(Pair("pow2_phase", nPow2Phase));
    rec.push_back(Pair("number_of_witnesses_raw", (uint64_t)nNumWitnessAddressesAll));
    rec.push_back(Pair("number_of_witnesses_total", (uint64_t)witInfo.witnessSelectionPoolUnfiltered.size()));
    rec.push_back(Pair("number_of_witnesses_eligible", (uint64_t)witInfo.witnessSelectionPoolFiltered.size()));
    rec.push_back(Pair("total_witness_weight_raw", (uint64_t)witInfo.nTotalWeightRaw));
    rec.push_back(Pair("total_witness_weight_eligible_raw", (uint64_t)witInfo.nTotalWeightEligibleRaw));
    rec.push_back(Pair("total_witness_weight_eligible_adjusted", (uint64_t)witInfo.nTotalWeightEligibleAdjusted));
    rec.push_back(Pair("selected_witness_address", sWitnessAddress));
    if (fVerbose)
    {
        UniValue averages(UniValue::VOBJ);
        {
            if (boost::accumulators::count(witnessWeightStats) > 0)
            {
                UniValue weight(UniValue::VOBJ);
                weight.push_back(Pair("largest", boost::accumulators::max(witnessWeightStats)));
                weight.push_back(Pair("smallest", boost::accumulators::min(witnessWeightStats)));
                weight.push_back(Pair("mean", boost::accumulators::mean(witnessWeightStats)));
                weight.push_back(Pair("median", boost::accumulators::median(witnessWeightStats)));
                averages.push_back(Pair("weight", weight));
            }
        }
        {
            if (boost::accumulators::count(witnessAmountStats) > 0)
            {
                UniValue amount(UniValue::VOBJ);
                amount.push_back(Pair("largest", ValueFromAmount(boost::accumulators::max(witnessAmountStats))));
                amount.push_back(Pair("smallest", ValueFromAmount(boost::accumulators::min(witnessAmountStats))));
                amount.push_back(Pair("mean", ValueFromAmount(boost::accumulators::mean(witnessAmountStats))));
                amount.push_back(Pair("median", ValueFromAmount(boost::accumulators::median(witnessAmountStats))));
                averages.push_back(Pair("amount", amount));
            }
        }
        {
            if (boost::accumulators::count(lockPeriodWeightStats) > 0)
            {
                UniValue lockPeriod(UniValue::VOBJ);
                lockPeriod.push_back(Pair("largest", boost::accumulators::max(lockPeriodWeightStats)));
                lockPeriod.push_back(Pair("smallest", boost::accumulators::min(lockPeriodWeightStats)));
                lockPeriod.push_back(Pair("mean", boost::accumulators::mean(lockPeriodWeightStats)));
                lockPeriod.push_back(Pair("median", boost::accumulators::median(lockPeriodWeightStats)));
                averages.push_back(Pair("lock_period", lockPeriod));
            }
        }
        {
            if (boost::accumulators::count(ageStats) > 0)
            {
                UniValue age(UniValue::VOBJ);
                age.push_back(Pair("largest", boost::accumulators::max(ageStats)));
                age.push_back(Pair("smallest", boost::accumulators::min(ageStats)));
                age.push_back(Pair("mean", boost::accumulators::mean(ageStats)));
                age.push_back(Pair("median", boost::accumulators::median(ageStats)));
                averages.push_back(Pair("age", age));
            }
        }
        rec.push_back(Pair("witness_statistics", averages));
        rec.push_back(Pair("witness_address_list", jsonAllWitnessAddresses));
    }
    witnessInfoForBlock.push_back(rec);

    return witnessInfoForBlock;
}

static UniValue disablewitnessing(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0) {
        throw std::runtime_error(
            "disablewitnessing\n"
            "\nStops all witnessing activity, call \"enablewitnessing\" to start witnessing again.\n"
        );
    }

    witnessingEnabled = false;
    return true;
}

static UniValue enablewitnessing(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0) {
        throw std::runtime_error(
            "enablewitnessing\n"
            "\nStarts all witnessing activity, call \"disablewitnessing\" to stop witnessing again.\n"
        );
    }

    witnessingEnabled = true;
    return true;
}

static UniValue dumpfiltercheckpoints(const JSONRPCRequest& request)
{
     if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "dumpfiltercheckpoints \"filename\"\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpfiltercheckpoints", ""));

    std::ofstream file;
    boost::filesystem::path filepath = request.params[0].get_str();
    filepath = boost::filesystem::absolute(filepath);
    file.open(filepath.string().c_str());
    if (!file.is_open())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");

    LogPrintf("Dumping filter checkpoints:\n");
    int nStart = IsArgSet("-testnet") ? 0 : 250000;//Earliest possible recovery phrase (before this we didn't use phrases)
    int nInterval1 = 500;
    int nInterval2 = 100;
    int nCrossOver = 500000;
    if (chainActive.Tip() != NULL)
    {
        LOCK2(cs_main, pactiveWallet->cs_wallet); // cs_main required for ReadBlockFromDisk.

        std::vector<unsigned char> allFilters;
        allFilters.reserve(3000000);
        int nMaxHeight = chainActive.Height();
        for (int i=nStart; i+nInterval2 < nMaxHeight;)
        {
            int nInterval = nInterval1;
            if (i >= nCrossOver)
                nInterval = nInterval2;

            RangedCPBlockFilter filter(chainActive[i-1], chainActive[i+nInterval-1]);
            const std::vector<unsigned char>& filterData = filter.GetEncodedFilter();
            {
                std::vector<unsigned char> filterSizeVector;
                CVectorWriter sizeWriter(0, 0, filterSizeVector, 0);
                WriteCompactSize(sizeWriter, filterData.size());
                std::copy(filterSizeVector.cbegin(), filterSizeVector.cend(), std::back_inserter(allFilters));
            }
            std::copy(filterData.cbegin(), filterData.cend(), std::back_inserter(allFilters));

            i += nInterval;
        }
        LogPrintf("size: %d\n", allFilters.size());
        file.write((const char*)&allFilters[0], allFilters.size());
    }

    file.close();
    return true;
}

static UniValue dumpdiffarray(const JSONRPCRequest& request)
{
    CWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "dumpdiffarray  ( height )\n"
            "\nDump code for a c++ array containing 'height' integers, where each integer represents the difficulty (nBits) of a block.\n"
            "\nThis mainly exists for testing and development purposes, and can be used to help verify that your client has not been tampered with.\n"
            "\nArguments:\n"
            "1. height     (numeric) The number of blocks to add to the array.\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpdiffarray 260000", ""));

    RPCTypeCheck(request.params, boost::assign::list_of(UniValue::VNUM));

    std::string reverseOutBuffer;
    std::string scratchBuffer;
    unsigned int nNumToOutput = request.params[0].get_int();
    reverseOutBuffer.reserve(16*nNumToOutput);

    CBlockIndex* pBlock = chainActive.Tip();
    while(pBlock->pprev && (unsigned int)pBlock->nHeight > nNumToOutput)
        pBlock = pBlock->pprev;

    int count=1;
    while(pBlock)
    {
        scratchBuffer = itostr(pBlock->nBits);
        std::reverse(scratchBuffer.begin(), scratchBuffer.end());
        if (count!= 1)
            reverseOutBuffer += " ,";
        reverseOutBuffer += scratchBuffer;
        if (count%10 == 0)
            reverseOutBuffer += "        \n";
        pBlock = pBlock->pprev;
        count=count+1;
    }

    std::reverse(reverseOutBuffer.begin(), reverseOutBuffer.end());
    LogPrintf("%s",reverseOutBuffer);

    return reverseOutBuffer;
}


static UniValue dumpblockgaps(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "dumpblockgaps  start_height count\n"
            "\nDump the block gaps for the last n blocks.\n"
            "\nArguments:\n"
            "1. start_height     (numeric) Where to start dumping from, counting backwards from chaintip.\n"
            "2. count           (numeric) The number of blocks to dump the block gaps of - going backwards from the start_height.\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpblockgaps 50", ""));

    RPCTypeCheck(request.params, boost::assign::list_of(UniValue::VNUM));

    int nStart = request.params[0].get_int();
    int nNumToOutput = request.params[1].get_int();

    CBlockIndex* pBlock = chainActive.Tip();

    UniValue jsonGaps(UniValue::VARR);

    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::median(boost::accumulators::with_p_square_quantile), boost::accumulators::tag::mean, boost::accumulators::tag::min, boost::accumulators::tag::max> > gapStats;

    while(pBlock && pBlock->pprev && --nStart>0)
    {
        pBlock = pBlock->pprev;
    }

    while(pBlock && pBlock->pprev && --nNumToOutput>0)
    {
        int64_t gap = std::abs((int64_t)pBlock->nTime - (int64_t)pBlock->pprev->nTime);
        pBlock = pBlock->pprev;
        if (gap > 6000)
        {
            continue;
        }
        jsonGaps.push_back(gap);
        gapStats(gap);
    }

    jsonGaps.push_back("max:");
    jsonGaps.push_back(boost::accumulators::max(gapStats));
    jsonGaps.push_back("min:");
    jsonGaps.push_back(boost::accumulators::min(gapStats));
    jsonGaps.push_back("mean:");
    jsonGaps.push_back(boost::accumulators::mean(gapStats));
    jsonGaps.push_back("median:");
    jsonGaps.push_back(boost::accumulators::median(gapStats));

    return jsonGaps;
}


static UniValue dumptransactionstats(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "dumptransactionstats start_height count\n"
            "\nDump the transaction stats for the last n blocks.\n"
            "\nArguments:\n"
            "1. start_height     (numeric) Where to start dumping from, counting backwards from chaintip.\n"
            "2. count           (numeric) The number of blocks to dump the block gaps of - going backwards from the start_height.\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpblockgaps 50", ""));

    RPCTypeCheck(request.params, boost::assign::list_of(UniValue::VNUM));

    int nStart = request.params[0].get_int();
    int nNumToOutput = request.params[1].get_int();

    CBlockIndex* pBlock = chainActive.Tip();

    UniValue jsonGaps(UniValue::VARR);

    while(pBlock && pBlock->pprev && --nStart>0)
    {
        pBlock = pBlock->pprev;
    }

    int count = 0;
    std::map<int64_t, int64_t> inputCount;
    std::map<int64_t, int64_t> outputCount;
    inputCount[1]=outputCount[1]=0;
    inputCount[2]=outputCount[2]=0;
    inputCount[3]=outputCount[3]=0;
    inputCount[4]=outputCount[4]=0;
    inputCount[5]=outputCount[5]=0;
    inputCount[6]=outputCount[6]=0;
    inputCount[7]=outputCount[7]=0;

    while(pBlock && pBlock->pprev && --nNumToOutput>0)
    {
        CBlock block;
        LOCK(cs_main);// cs_main lock required for ReadBlockFromDisk
        if (ReadBlockFromDisk(block, pBlock, Params()))
        {
            for (auto transaction : block.vtx)
            {
                ++count;
                if (transaction->vin.size() >=7)
                {
                    ++inputCount[7];
                }
                else
                {
                    ++inputCount[transaction->vin.size()];
                }
                if (transaction->vout.size() >=7)
                {
                    ++outputCount[7];
                }
                else
                {
                    ++outputCount[transaction->vout.size()];
                }
            }
        }
        pBlock = pBlock->pprev;
    }

    jsonGaps.push_back("count:");
    jsonGaps.push_back(count);
    jsonGaps.push_back("1:");
    jsonGaps.push_back(inputCount[1]);
    jsonGaps.push_back(outputCount[1]);
    jsonGaps.push_back("2:");
    jsonGaps.push_back(inputCount[2]);
    jsonGaps.push_back(outputCount[2]);
    jsonGaps.push_back("3:");
    jsonGaps.push_back(inputCount[3]);
    jsonGaps.push_back(outputCount[3]);
    jsonGaps.push_back("4:");
    jsonGaps.push_back(inputCount[4]);
    jsonGaps.push_back(outputCount[4]);
    jsonGaps.push_back("5:");
    jsonGaps.push_back(inputCount[5]);
    jsonGaps.push_back(outputCount[5]);
    jsonGaps.push_back("6:");
    jsonGaps.push_back(inputCount[6]);
    jsonGaps.push_back(outputCount[6]);
    jsonGaps.push_back("n:");
    jsonGaps.push_back(inputCount[7]);
    jsonGaps.push_back(outputCount[7]);


    return jsonGaps;
}


static UniValue changeaccountname(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "changeaccountname \"account\" \"name\"\n"
            "\nChange the name of an account.\n"
            "\nArguments:\n"
            "1. \"account\"         (string) The UUID or unique label of the account.\n"
            "2. \"name\"           (string) The new label for the account.\n"
            "\nResult:\n"
            "\nReturn the final label for the account. Note this may be different from the given label in the case of duplicates.\n"
            "\nActive account is used as the default for all commands that take an optional account argument.\n"
            "\nExamples:\n"
            + HelpExampleCli("changeaccountname \"My witness account\" \"Charity donations\"", "")
            + HelpExampleRpc("changeaccountname \"My witness account\" \"Charity donations\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CAccount* account = AccountFromValue(pwallet, request.params[0], false);
    std::string label = request.params[1].get_str();

    pwallet->changeAccountName(account, label);

    return account->getLabel();
}

#define MINIMUM_VALUABLE_AMOUNT 1000000000
static UniValue deleteaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "deleteaccount \"account\" \"force\"\n"
            "\nDelete an account.\n"
            "\nArguments:\n"
            "1. \"account\"        (string) The UUID or unique label of the account.\n"
            "2. \"force\"          (string) Specify string force to force deletion of a non-empty account.\n"
            "\nExamples:\n"
            + HelpExampleCli("deleteaccount \"My account\"", "")
            + HelpExampleRpc("deleteaccount \"My account\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CAccount* account = AccountFromValue(pwallet, request.params[0], false);

    bool forcePurge = false;
    if (account->IsPoW2Witness() && account->IsFixedKeyPool())
        forcePurge = true;
    if (request.params.size() == 1 || request.params[1].get_str() != "force")
    {
        boost::uuids::uuid accountUUID = account->getUUID();
        CAmount balance = pwallet->GetLegacyBalance(ISMINE_SPENDABLE, 0, &accountUUID );
        if (account->IsPoW2Witness() && account->IsFixedKeyPool())
        {
            balance = pwallet->GetBalance(account, true, false, true);
        }
        if (balance > MINIMUM_VALUABLE_AMOUNT && !account->IsReadOnly())
        {
            throw std::runtime_error("Account not empty, please first empty your account before trying to delete it.");
        }
    }

    CWalletDB walletdb(*pwallet->dbw);
    pwallet->deleteAccount(walletdb, account, forcePurge);
    return true;
}


static UniValue createaccounthelper(CWallet* pwallet, std::string accountName, std::string accountType, bool bMakeActive=true)
{
    CAccount* account = NULL;

    EnsureWalletIsUnlocked(pwallet);

    if (accountType == "HD")
    {
        account = pwallet->GenerateNewAccount(accountName, AccountState::Normal, AccountType::Desktop, bMakeActive);
    }
    else if (accountType == "Mobile")
    {
        account = pwallet->GenerateNewAccount(accountName.c_str(), AccountState::Normal, AccountType::Mobi, bMakeActive);
    }
    else if (accountType == "Witness")
    {
        account = pwallet->GenerateNewAccount(accountName.c_str(), AccountState::Normal, AccountType::PoW2Witness, bMakeActive);
    }
    else if (accountType == "Legacy")
    {
        account = pwallet->GenerateNewLegacyAccount(accountName.c_str());
    }

    if (!account)
        throw std::runtime_error("Unable to create account.");

    return getUUIDAsString(account->getUUID());
}

static UniValue createaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() == 0 || request.params.size() > 2)
        throw std::runtime_error(
            "createaccount \"name\" \"type\"\n"
            "Create an account, for HD accounts the currently active seed will be used to create the account.\n"
            "\nArguments:\n"
            "1. \"name\"       (string) Specify the label for the account.\n"
            "2. \"type\"       (string, optional) Type of account to create (HD; Mobile; Legacy; Witness)\n"
            "\nExamples:\n"
            + HelpExampleCli("createaccount \"My new account\"", "")
            + HelpExampleRpc("createaccount \"My new account\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");


    std::string accountType = "HD";
    if (request.params.size() > 1)
    {
        accountType = request.params[1].get_str();
        if (accountType != "HD" && accountType != "Mobile" && accountType != "Legacy" && accountType != "Witness")
            throw std::runtime_error("Invalid account type");
    }

    return createaccounthelper(pwallet, request.params[0].get_str(), accountType);
}


static UniValue createwitnessaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "createwitnessaccount \"name\"\n"
            "Create an account, the currently active seed will be used to create the account.\n"
            "\nArguments:\n"
            "1. \"name\"       (string) Specify the label for the account.\n"
            "\nExamples:\n"
            + HelpExampleCli("createwitnessaccount \"My 3y savings\"", "")
            + HelpExampleRpc("createwitnessaccount \"My 3y savings\"", ""));

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    if (GetPoW2Phase(chainActive.Tip(), Params(), chainActive) < 2)
        throw std::runtime_error("Cannot create witness accounts before phase 2 activates.");

    return createaccounthelper(pwallet, request.params[0].get_str(), "Witness", false);
}

static std::vector<std::tuple<CTxOut, uint64_t, COutPoint>> getCurrentOutputsForWitnessAddress(CGuldenAddress& searchAddress)
{
    std::map<COutPoint, Coin> allWitnessCoins;
    if (!getAllUnspentWitnessCoins(chainActive, Params(), chainActive.Tip(), allWitnessCoins))
        throw std::runtime_error("Failed to enumerate all witness coins.");

    std::vector<std::tuple<CTxOut, uint64_t, COutPoint>> matchedOutputs;
    for (const auto& [outpoint, coin] : allWitnessCoins)
    {
        CTxDestination compareDestination;
        bool fValidAddress = ExtractDestination(coin.out, compareDestination);

        if (fValidAddress && (CGuldenAddress(compareDestination) == searchAddress))
        {
            matchedOutputs.push_back(std::tuple(coin.out, coin.nHeight, outpoint));
        }
    }
    return matchedOutputs;
}

static std::vector<std::tuple<CTxOut, uint64_t, COutPoint>> getCurrentOutputsForWitnessAccount(CAccount* forAccount)
{
    std::map<COutPoint, Coin> allWitnessCoins;
    if (!getAllUnspentWitnessCoins(chainActive, Params(), chainActive.Tip(), allWitnessCoins))
        throw std::runtime_error("Failed to enumerate all witness coins.");

    std::vector<std::tuple<CTxOut, uint64_t, COutPoint>> matchedOutputs;
    for (const auto& [outpoint, coin] : allWitnessCoins)
    {
        if (IsMine(*forAccount, coin.out))
        {
            matchedOutputs.push_back(std::tuple(coin.out, coin.nHeight, outpoint));
        }
    }
    return matchedOutputs;
}

//! Given a string specifier, calculate a lock length in blocks to match it. e.g. 1d -> 576; 5b -> 5; 1m -> 17280
//! Returns 0 if specifier is invalid.
static uint64_t GetLockPeriodInBlocksFromFormattedStringSpecifier(std::string formattedLockPeriodSpecifier)
{
    uint64_t lockPeriodInBlocks = 0;
    int nMultiplier = 1;
    if (boost::algorithm::ends_with(formattedLockPeriodSpecifier, "y"))
    {
        nMultiplier = 365 * 576;
        formattedLockPeriodSpecifier.pop_back();
    }
    else if (boost::algorithm::ends_with(formattedLockPeriodSpecifier, "m"))
    {
        nMultiplier = 30 * 576;
        formattedLockPeriodSpecifier.pop_back();
    }
    else if (boost::algorithm::ends_with(formattedLockPeriodSpecifier, "w"))
    {
        nMultiplier = 7 * 576;
        formattedLockPeriodSpecifier.pop_back();
    }
    else if (boost::algorithm::ends_with(formattedLockPeriodSpecifier, "d"))
    {
        nMultiplier = 576;
        formattedLockPeriodSpecifier.pop_back();
    }
    else if (boost::algorithm::ends_with(formattedLockPeriodSpecifier, "b"))
    {
        nMultiplier = 1;
        formattedLockPeriodSpecifier.pop_back();
    }
    if (!ParseUInt64(formattedLockPeriodSpecifier, &lockPeriodInBlocks))
        return 0;
    lockPeriodInBlocks *=  nMultiplier;
    return lockPeriodInBlocks;
}

static UniValue fundwitnessaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() < 4 || request.params.size() > 5)
        throw std::runtime_error(
            "fundwitnessaccount \"funding_account\" \"witness_account\" \"amount\" \"time\" \"force_multiple\" \n"
            "Lock \"amount\" NLG in \"witness_account\" for time period \"time\" using funds from \"funding_account\"\n"
            "NB! Though it is possible to fund a witness account that already has a balance, this can cause UI issues and is not strictly supported.\n"
            "It is highly recommended to rather use 'extendwitnessaccount' in this case which behaves more like what most people would expect.\n"
            "By default this command will fail if an account already contains an existing funded address.\n"
            "Note that this command is not currently calendar aware, it performs simplistic conversion i.e. 1 month is 30 days. This may change in future.\n"
            "\nArguments:\n"
            "1. \"funding_account\"      (string, required) The unique UUID or label for the account from which money will be removed. Use \"\" for the active account or \"*\" for all accounts to be considered.\n"
            "2. \"witness_account\"      (string, required) The unique UUID or label for the witness account that will hold the locked funds.\n"
            "3. \"amount\"               (string, required) The amount of NLG to hold locked in the witness account. Minimum amount of 5000 NLG is allowed.\n"
            "4. \"time\"                 (string, required) The time period for which the funds should be locked in the witness account. Minimum of 1 month and a maximum of 3 years. By default this is interpreted as blocks e.g. \"1000\", suffix with \"y\", \"m\", \"w\", \"d\", \"b\" to specifically work in years, months, weeks, days or blocks.\n"
            "5. force_multiple         (boolean, optional, default=false) Allow funding an account that already contains a valid witness address. \n"
            "\nResult:\n"
            "[\n"
            "     \"txid\":\"txid\",   (string) The txid of the created transaction\n"
            "     \"fee_amount\":n   (number) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            "\nTake 10000NLG out of \"mysavingsaccount\" and lock in \"mywitnessaccount\" for 2 years.\n"
            + HelpExampleCli("fundwitnessaccount \"mysavingsaccount\" \"mywitnessaccount\" \"10000\" \"2y\"", "")
            + "\nTake 10000NLG out of \"mysavingsaccount\" and lock in \"mywitnessaccount\" for 2 months.\n"
            + HelpExampleCli("fundwitnessaccount \"mysavingsaccount\" \"mywitnessaccount\" \"10000\" \"2m\"", "")
            + "\nTake 10000NLG out of \"mysavingsaccount\" and lock in \"mywitnessaccount\" for 100 days.\n"
            + HelpExampleCli("fundwitnessaccount \"mysavingsaccount\" \"mywitnessaccount\" \"10000\" \"100d\"", "")
            + HelpExampleRpc("fundwitnessaccount \"mysavingsaccount\" \"mywitnessaccount\" \"10000\" \"2y\"", ""));

    int nPoW2TipPhase = GetPoW2Phase(chainActive.Tip(), Params(), chainActive);

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (nPoW2TipPhase < 2)
        throw std::runtime_error("Cannot fund witness accounts before phase 2 activates.");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], true);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' account.
    CAccount* targetWitnessAccount = AccountFromValue(pwallet, request.params[1], true);
    if (!targetWitnessAccount)
        throw std::runtime_error(strprintf("Unable to locate witness account [%s].",  request.params[1].get_str()));

    bool fAllowMultiple = false;
    if (request.params.size() >= 5)
        fAllowMultiple = request.params[4].get_bool();

    // arg3 - amount
    CAmount nAmount =  AmountFromValue(request.params[2]);

    // arg4 - lock period.
    // Calculate lock period based on suffix (if one is present) otherwise leave as is.
    std::string formattedLockPeriodSpecifier = request.params[3].getValStr();
    uint64_t nLockPeriodInBlocks = GetLockPeriodInBlocksFromFormattedStringSpecifier(formattedLockPeriodSpecifier);

    try {
        std::string txid;
        CAmount fee;
        fundwitnessaccount(pwallet, fundingAccount, targetWitnessAccount, nAmount, nLockPeriodInBlocks, fAllowMultiple, &txid, &fee);
        UniValue result(UniValue::VOBJ);
        result.push_back(Pair("txid", txid));
        result.push_back(Pair("fee_amount", ValueFromAmount(fee)));
        return result;
    }
    catch (witness_error& e) {
        throw JSONRPCError(e.code(), e.what());
    }
    catch (std::runtime_error& e) {
        throw JSONRPCError(RPC_MISC_ERROR, e.what());
    }
}

static UniValue extendwitnessaddress(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 4)
        throw std::runtime_error(
            "extendwitnessaddress \"funding_account\" \"witness_address\" \"amount\" \"time\" \n"
            "Change the currently locked amount and time period for \"witness_address\" to match the new \"amount\" and time period \"time\"\n"
            "Note the new amount must be ≥ the old amount, and the new time period must exceed the remaining lock period that is currently set on the account\n"
            "\"funding_account\" is the account from which the locked funds will be claimed.\n"
            "\"time\" may be a minimum of 1 month and a maximum of 3 years.\n"
            "\nArguments:\n"
            "1. \"funding_account\"  (string, required) The unique UUID or label for the account from which money will be removed.\n"
            "2. \"witness_address\"  (string, required) The Gulden address for the witness key.\n"
            "3. \"amount\"           (string, required) The amount of NLG to hold locked in the witness account. Minimum amount of 5000 NLG is allowed.\n"
            "4. \"time\"             (string, required) The time period for which the funds should be locked in the witness account. By default this is interpreted as blocks e.g. \"1000\", suffix with \"y\", \"m\", \"w\", \"d\", \"b\" to specifically work in years, months, weeks, days or blocks.\n"
            "\nResult:\n"
            "[\n"
            "     \"txid\":\"txid\",   (string) The txid of the created transaction\n"
            "     \"fee_amount\":n   (number) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("extendwitnessaddress \"My account\" \"2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN\" \"50000\" \"2y\"", "")
            + HelpExampleRpc("extendwitnessaddress \"My account\" \"2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN\" \"50000\" \"2y\"", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' address.
    CGuldenAddress witnessAddress(request.params[1].get_str());
    bool isValid = witnessAddress.IsValidWitness(Params());

    if (!isValid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Not a valid witness address [%s].", request.params[1].get_str()));

    const auto& unspentWitnessOutputs = getCurrentOutputsForWitnessAddress(witnessAddress);
    if (unspentWitnessOutputs.size() == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Address does not contain any witness outputs [%s].", request.params[1].get_str()));

    // arg3 - amount
    CAmount requestedAmount =  AmountFromValue(request.params[2]);
    if (requestedAmount < (gMinimumWitnessAmount*COIN))
        throw JSONRPCError(RPC_TYPE_ERROR, strprintf("Witness amount must be %d or larger", gMinimumWitnessAmount));

    // arg4 - lock period.
    // Calculate lock period based on suffix (if one is present) otherwise leave as is.
    std::string formattedLockPeriodSpecifier = request.params[3].getValStr();
    uint64_t requestedLockPeriodInBlocks = GetLockPeriodInBlocksFromFormattedStringSpecifier(formattedLockPeriodSpecifier);
    if (requestedLockPeriodInBlocks == 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid number passed for lock period.");

    try {
        std::string txid;
        CAmount fee;
        extendwitnessaddresshelper(fundingAccount, unspentWitnessOutputs, pwallet, requestedAmount, requestedLockPeriodInBlocks, &txid, &fee);
        UniValue result(UniValue::VOBJ);
        result.push_back(Pair("txid", txid));
        result.push_back(Pair("fee_amount", ValueFromAmount(fee)));
        return result;
    }
    catch (witness_error& e) {
        throw JSONRPCError(e.code(), e.what());
    }
    catch (std::runtime_error& e) {
        throw JSONRPCError(RPC_MISC_ERROR, e.what());
    }
}

static UniValue extendwitnessaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 4)
        throw std::runtime_error(
            "extendwitnessaccount \"funding_account\" \"witness_account\" \"amount\" \"time\" \n"
            "Change the currently locked amount and time period for \"witness_account\" to match the new \"amount\" ant time period \"time\"\n"
            "Note the new amount must be ≥ the old amount, and the new time period must exceed the remaining lock period that is currently set on the account\n"
            "\"funding_account\" is the account from which the locked funds will be claimed.\n"
            "\"time\" may be a minimum of 1 month and a maximum of 3 years.\n"
            "\nArguments:\n"
            "1. \"funding_account\" (string, required) The unique UUID or label for the account from which money will be removed.\n"
            "2. \"witness_account\" (string, required) The unique UUID or label for the witness account that will hold the locked funds.\n"
            "3. \"amount\"          (string, required) The amount of NLG to hold locked in the witness account. Minimum amount of 5000 NLG is allowed.\n"
            "4. \"time\"            (string, required) The time period for which the funds should be locked in the witness account. By default this is interpreted as blocks e.g. \"1000\", suffix with \"y\", \"m\", \"w\", \"d\", \"b\" to specifically work in years, months, weeks, days or blocks.\n"
            "\nResult:\n"
            "[\n"
            "     \"txid\":\"txid\",  (string) The txid of the created transaction\n"
            "     \"fee_amount\":n  (string) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("extendwitnessaccount \"My account\" \"My witness account\" \"50000\" \"2y\"", "")
            + HelpExampleRpc("extendwitnessaccount \"My account\" \"My witness account\" \"50000\" \"2y\"", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' account.
    CAccount* witnessAccount = AccountFromValue(pwallet, request.params[1], false);
    if (!witnessAccount)
        throw std::runtime_error(strprintf("Unable to locate witness account [%s].",  request.params[1].get_str()));

    // arg3 - amount
    CAmount requestedAmount =  AmountFromValue(request.params[2]);
    if (requestedAmount < (gMinimumWitnessAmount*COIN))
        throw JSONRPCError(RPC_TYPE_ERROR, strprintf("Witness amount must be %d or larger", gMinimumWitnessAmount));

    // arg4 - lock period.
    // Calculate lock period based on suffix (if one is present) otherwise leave as is.
    std::string formattedLockPeriodSpecifier = request.params[3].getValStr();
    uint64_t requestedLockPeriodInBlocks = GetLockPeriodInBlocksFromFormattedStringSpecifier(formattedLockPeriodSpecifier);
    if (requestedLockPeriodInBlocks == 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid number passed for lock period.");

    try {
        std::string txid;
        CAmount fee;
        extendwitnessaccount(pwallet, fundingAccount, witnessAccount, requestedAmount, requestedLockPeriodInBlocks, &txid, &fee);
        UniValue result(UniValue::VOBJ);
        result.push_back(Pair("txid", txid));
        result.push_back(Pair("fee_amount", ValueFromAmount(fee)));
        return result;
    }
    catch (witness_error& e) {
        throw JSONRPCError(e.code(), e.what());
    }
    catch (std::runtime_error& e) {
        throw JSONRPCError(RPC_MISC_ERROR, e.what());
    }
}

static UniValue getactiveaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getactiveaccount \n"
            "\nReturn the UUID for the currently active account.\n"
            "\nActive account is used as the default for all commands that take an optional account argument.\n"
            "\nExamples:\n"
            + HelpExampleCli("getactiveaccount", "")
            + HelpExampleRpc("getactiveaccount", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    if (!pwallet->activeAccount)
        throw std::runtime_error("No account active");

    return getUUIDAsString(pwallet->activeAccount->getUUID());
}

static UniValue getreadonlyaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getreadonlyaccount \"account\" \n"
            "\nGet the public key of an HD account, this can be used to import the account as a read only account in another wallet.\n"
            "1. \"account\"        (required) The unique UUID or label for the account .\n"
            "\nResult:\n"
            "\nReturn the public key as an encoded string, that can be used with the \"importreadonlyaccount\" command.\n"
            "\nNB! it is important to be careful with and protect access to this public key as if it is compromised it can compromise security of your entire wallet, in cases where one or more child private keys are also compromised.\n"
            "\nExamples:\n"
            + HelpExampleCli("getreadonlyaccount \"My account\"", "")
            + HelpExampleRpc("getreadonlyaccount \"My account\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CAccount* account = AccountFromValue(pwallet, request.params[0], false);

    if (!account->IsHD())
        throw std::runtime_error("Can only be used on a HD account.");

    CAccountHD* accountHD = dynamic_cast<CAccountHD*>(account);

    EnsureWalletIsUnlocked(pwallet);

    return accountHD->GetAccountMasterPubKeyEncoded().c_str();
}

static UniValue importreadonlyaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "importreadonlyaccount \"name\" \"encodedkey\" \n"
            "\nImport a read only account from an \"encodedkey\" which has been obtained by using \"getreadonlyaccount\"\n"
            "1. \"name\"       (string) Name to assign to the new account.\n"
            "2. \"encodedkey\" (string) Encoded string containing the extended public key for the account.\n"
            "\nResult:\n"
            "\nReturn the UUID of the new account.\n"
            "\nExamples:\n"
            + HelpExampleCli("importreadonlyaccount \"Watch account\" \"dd3tNdQ8A4KqYvYVvXzGEU7ChdNye9RdTixnLSFqpQHG-2Rakbbkn7GDUTdD6wtSd5KV5PnCFgQt3FPc8eYkMonRM\"", "")
            + HelpExampleRpc("importreadonlyaccount \"Watch account\" \"dd3tNdQ8A4KqYvYVvXzGEU7ChdNye9RdTixnLSFqpQHG-2Rakbbkn7GDUTdD6wtSd5KV5PnCFgQt3FPc8eYkMonRM\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    EnsureWalletIsUnlocked(pwallet);

    CAccount* account = pwallet->CreateReadOnlyAccount(request.params[0].get_str().c_str(), request.params[1].get_str().c_str());

    if (!account)
        throw std::runtime_error("Unable to create account.");

    //fixme: (PHASE5) Use a timestamp here
    // Whenever a key is imported, we need to scan the whole chain - do so now
    pwallet->nTimeFirstKey = 1;
    ResetSPVStartRescanThread();

    return getUUIDAsString(account->getUUID());
}



static UniValue importlinkedaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "importlinkedaccount \"name\" \"encoded_key_uri\" \n"
            "\nImport a linked account from an \"encoded_key_uri\"\n"
            "1. \"name\"       (string) Name to assign to the new account.\n"
            "2. \"encoded_key_uri\" (string) Encoded string containing the extended public key for the account.\n"
            "\nResult:\n"
            "\nReturn the UUID of the new account.\n"
            "\nExamples:\n"
            + HelpExampleCli("importlinkedaccount \"Linked account\" \"guldensync:2JQWMRUmrVym8Ak8TdeLhveAXkA1a5fb9fWzQUZkhd8G-3tS68yeav8TRJqhf5NEsa44tLRyjRouZQCcwcQ4Q5CSe:3mM4jYg7L4FhLC;TNhC2TDsD2L2PW7ri7ysn9YTfoQfpWT1K3\"", "")
            + HelpExampleRpc("importlinkedaccount \"Linked account\" \"guldensync:2JQWMRUmrVym8Ak8TdeLhveAXkA1a5fb9fWzQUZkhd8G-3tS68yeav8TRJqhf5NEsa44tLRyjRouZQCcwcQ4Q5CSe:3mM4jYg7L4FhLC;TNhC2TDsD2L2PW7ri7ysn9YTfoQfpWT1K3\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    EnsureWalletIsUnlocked(pwallet);

    CGuldenSecretExt<CExtKey> linkedKey;
    if (!linkedKey.fromURIString(request.params[1].get_str().c_str()))
    {
        return false;
    }
    CAccount* account =  pwallet->CreateSeedlessHDAccount(request.params[0].get_str().c_str(), linkedKey.getKeyRaw(), AccountState::Normal, AccountType::Mobi);

    if (!account)
        throw std::runtime_error("Unable to create account.");

    ResetSPVStartRescanThread();

    return getUUIDAsString(account->getUUID());
}

static UniValue getactiveseed(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getactiveseed \n"
            "\nReturn the UUID for the currently active account.\n"
            "\nActive account is used as the default for all commands that take an optional account argument.\n"
            "\nExamples:\n"
            + HelpExampleCli("getactiveseed", "")
            + HelpExampleRpc("getactiveseed", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    if (!pwallet->activeSeed)
        throw std::runtime_error("No seed active");

    return getUUIDAsString(pwallet->activeSeed->getUUID());
}

static UniValue setactiveaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "setactiveaccount \n"
            "\nSet the currently active account based on name or uuid.\n"
            "1. \"account\"        (string, required) The unique UUID or label for the account or \"\" for the active account.\n"
            "\nExamples:\n"
            + HelpExampleCli("setactiveaccount \"My account\"", "")
            + HelpExampleRpc("setactiveaccount \"My account\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CAccount* account = AccountFromValue(pwallet, request.params[0], false);

    CWalletDB walletdb(*pwallet->dbw);
    pwallet->setActiveAccount(walletdb, account);
    return getUUIDAsString(account->getUUID());
}

static UniValue getaccountbalances(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 2)
        throw std::runtime_error(
            "getaccountbalances ( min_conf include_watchonly )\n"
            "Returns a list of balances for all accounts in the wallet.\n"
            "\nArguments:\n"
            "1. minconf           (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "2. include_watchonly (boolean, optional, default=false) Also include balance in watch-only addresses (see 'importaddress')\n"
            "\nResult:\n"
            "amount              (numeric) The total amount in " + CURRENCY_UNIT + " received for this account.\n"
            "\nExamples:\n"
            "\nThe total amount in the wallet\n"
            + HelpExampleCli("getaccountbalances", "") +
            "\nThe total amount in the wallet at least 5 blocks confirmed\n"
            + HelpExampleCli("getaccountbalances", "6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getaccountbalances", "6")
        );

    DS_LOCK2(cs_main, pwallet->cs_wallet);

    int nMinDepth = 1;
    if (request.params.size() > 0)
        nMinDepth = request.params[0].get_int();

    bool includeWatchOnly = false;
    if (request.params.size() > 1)
        includeWatchOnly = request.params[1].get_bool();

    UniValue allAccounts(UniValue::VARR);

    //NB! - Intermediate AccountFromValue step is required in order to handle default account semantics.
    for (const auto& accountPair : pwallet->mapAccounts)
    {
        UniValue rec(UniValue::VOBJ);
        rec.push_back(Pair("UUID", getUUIDAsString(accountPair.first)));
        rec.push_back(Pair("label", accountPair.second->getLabel()));
        rec.push_back(Pair("balance", ValueFromAmount(pwallet->GetLegacyBalance(includeWatchOnly?ISMINE_ALL:ISMINE_SPENDABLE, nMinDepth, &accountPair.first))));
        allAccounts.push_back(rec);
    }

    return allAccounts;
}

static CHDSeed* SeedFromValue(CWallet* pwallet, const UniValue& value, bool useDefaultIfEmpty)
{
    std::string strSeedUUID = value.get_str();

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    if (strSeedUUID.empty())
    {
        if (!useDefaultIfEmpty || !pwallet->getActiveSeed())
        {
            throw std::runtime_error("No seed identifier passed, and no active seed selected, please select an active seed or pass a valid identifier.");
        }
        return pwallet->getActiveSeed();
    }

    boost::uuids::uuid seedUUID = getUUIDFromString(strSeedUUID);

    CHDSeed* foundSeed = NULL;
    if (pwallet->mapSeeds.find(seedUUID) != pwallet->mapSeeds.end())
    {
        foundSeed = pwallet->mapSeeds[seedUUID];
    }

    if (!foundSeed)
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Not a valid seed UUID.");

    return foundSeed;
}

static UniValue setactiveseed(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "setactiveseed \n"
            "\nSet the currently active seed by UUID.\n"
            "\nExamples:\n"
            + HelpExampleCli("setactiveseed \"827f0000-0300-0000-0000-000000000000\"", "")
            + HelpExampleRpc("setactiveseed \"827f0000-0300-0000-0000-000000000000\"", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CHDSeed* seed = SeedFromValue(pwallet, request.params[0], false);

    CWalletDB walletdb(*pwallet->dbw);
    pwallet->setActiveSeed(walletdb, seed);
    return getUUIDAsString(seed->getUUID());
}


static UniValue createseed(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() > 1)
        throw std::runtime_error(
            "createseed \n"
            "\nCreate a new seed using random entropy.\n"
            "1. \"type\"       (string, optional default=BIP44) Type of seed to create (BIP44; BIP44NH; BIP44E; BIP32; BIP32L)\n"
            "\nThe default is correct in almost all cases, only experts should work with the other types\n"
            "\nBIP44 - This is the standard Gulden seed type that should be used in almost all cases.\n"
            "\nBIP44NH - (No Hardening) This is the same as above, however with weakened security required for \"read only\" (watch) seed capability, use this only if you understand the implications and if you want to share your seed with another read only wallet.\n"
            "\nBIP44E - This is a modified BIP44 with a different hash value, required for compatibility with some external wallets (e.g. Coinomi).\n"
            "\nBIP32 - Older HD standard that was used by our mobile wallets before 1.6.0, use this to import/recover old mobile recovery phrases.\n"
            "\nBIP32L - (Legacy) Even older HD standard that was used by our first android wallets, use this to import/recover very old mobile recovery phrases.\n"
            "\nResult:\n"
            "\nReturn the UUID of the new seed.\n"
            "\nExamples:\n"
            + HelpExampleCli("createseed", "")
            + HelpExampleRpc("createseed", ""));



    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    EnsureWalletIsUnlocked(pwallet);

    CHDSeed::SeedType seedType = CHDSeed::CHDSeed::BIP44;
    if (request.params.size() > 0)
    {
        seedType = SeedTypeFromString(request.params[0].get_str());
    }

    CHDSeed* newSeed = pwallet->GenerateHDSeed(seedType);

    if(!newSeed)
        throw std::runtime_error("Failed to generate seed");

    return getUUIDAsString(newSeed->getUUID());
}


static UniValue deleteseed(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "deleteseed \"seed\" should_purge_accounts \n" 
            "\nDelete a HD seed.\n"
            "1. \"seed\"                (string, required) The unique UUID for the seed that we want mnemonics of, or \"\" for the active seed.\n"
            "2. should_purge_accounts     (boolean, optional, default=false) Permanently purge from the wallet, all accounts associated with the seed, as opposed to simply marking them as deleted which places them in a hidden but still accessible state."
            "\nResult:\n"
            "\ntrue on success.\n"
            "\nExamples:\n"
            + HelpExampleCli("deleteseed \"827f0000-0300-0000-0000-000000000000\"", "")
            + HelpExampleRpc("deleteseed \"827f0000-0300-0000-0000-000000000000\"", ""));


    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CHDSeed* seed = SeedFromValue(pwallet, request.params[0], true);

    EnsureWalletIsUnlocked(pwallet);

    bool shouldPurgeAccounts = false;
    if (request.params.size() > 1)
        shouldPurgeAccounts = request.params[1].get_bool();

    CWalletDB walletdb(*pwallet->dbw);
    pwallet->DeleteSeed(walletdb, seed, shouldPurgeAccounts);

    return true;
}

static UniValue importseed(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 3)
        throw std::runtime_error(
            "importseed \"mnemonic_or_pubkey\" \"type\" is_read_only \n"
            "\nSet the currently active seed by UUID.\n"
            "1. \"mnemonic_or_pubkey\"       (string) Specify the BIP44 mnemonic that will be used to generate the seed.\n"
            "2. \"type\"       (string, optional default=BIP44) Type of seed to create (BIP44; BIP44NH; BIP44E; BIP32; BIP32L)\n"
            "\nThe default is correct in almost all cases, only experts should work with the other types\n"
            "\nBIP44 - This is the standard Gulden seed type that should be used in almost all cases.\n"
            "\nBIP44NH - (No Hardening) This is the same as above, however with weakened security required for \"read only\" (watch) seed capability, use this only if you understand the implications and if you want to share your seed with another read only wallet.\n"
            "\nBIP44E - This is a modified BIP44 with a different hash value, required for compatibility with some external wallets (e.g. Coinomi).\n"
            "\nBIP32 - Older HD standard that was used by our mobile wallets before 1.6.0, use this to import/recover old mobile recovery phrases.\n"
            "\nBIP32L - (Legacy) Even older HD standard that was used by our first android wallets, use this to import/recover very old mobile recovery phrases.\n"
            "\nIn the case of read only seeds a pubkey rather than a mnemonic is required.\n"
            "3. is_read_only      (boolean, optional, default=false) Account is a 'read only account' - type argument will be ignored and always set to BIP44NH in this case. Wallet will be rescanned for transactions.\n"
            "\nResult:\n"
            "\nReturn the UUID of the new seed.\n"
            "\nExamples:\n"
            + HelpExampleCli("importseed \"green cliff good ghost orange charge cancel blue group interest walk yellow\"", "")
            + HelpExampleRpc("importseed \"green cliff good ghost orange charge cancel blue group interest walk yellow\"", ""));

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    EnsureWalletIsUnlocked(pwallet);

    CHDSeed::SeedType seedType = CHDSeed::CHDSeed::BIP44;
    if (request.params.size() > 1)
    {
        seedType = SeedTypeFromString(request.params[1].get_str());
    }

    bool fReadOnly = false;
    if (request.params.size() > 2)
        fReadOnly = request.params[2].get_bool();

    CHDSeed* newSeed = NULL;
    if (fReadOnly)
    {
        SecureString pubkeyString = request.params[0].get_str().c_str();
        newSeed = pwallet->ImportHDSeedFromPubkey(pubkeyString);
    }
    else
    {
        SecureString mnemonic = request.params[0].get_str().c_str();
        newSeed = pwallet->ImportHDSeed(mnemonic, seedType);
    }

    //fixme: (POST-PHASE5) Use a timestamp here
    // Whenever a key is imported, we need to scan the whole chain - do so now
    pwallet->nTimeFirstKey = 1;
    ResetSPVStartRescanThread();

    return getUUIDAsString(newSeed->getUUID());
}

static UniValue listallaccounts(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 2)
        throw std::runtime_error(
            "listaccounts \"seed\" \"state\"\n"
            "\nArguments:\n"
            "1. \"seed\"        (string, optional) The unique UUID for the seed that we want accounts of.\n"
            "2. \"state\"       (string, optional, default=Normal) The state of account to list, options are: Normal, Deleted, Shadow, ShadowChild. \"*\" or \"\" to list all account states.\n"
            "\nResult:\n"
            "\nReturn the UUID and label for all wallet accounts.\n"
            "\nNote UUID is guaranteed to be unique while label is not.\n"
            "\nExamples:\n"
            + HelpExampleCli("listaccounts \"827f0000-0300-0000-0000-000000000000\"", "")
            + HelpExampleRpc("listaccounts \"827f0000-0300-0000-0000-000000000000\"", ""));

    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    std::string sStateSearch = "*";

    CHDSeed* forSeed = NULL;
    if (request.params.size() > 0 && request.params[0].get_str() != "*")
        forSeed = SeedFromValue(pwallet, request.params[0], true);
    if (request.params.size() > 1)
        sStateSearch = request.params[1].get_str();
    if (sStateSearch.empty())
        sStateSearch = "*";

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    UniValue allAccounts(UniValue::VARR);

    for (const auto& accountPair : pwallet->mapAccounts)
    {
        if (sStateSearch != "*" && sStateSearch != GetAccountStateString(accountPair.second->m_State))
            continue;

        if (!accountPair.second->IsHD())
        {
            if (!forSeed)
            {
                UniValue rec(UniValue::VOBJ);
                rec.push_back(Pair("UUID", getUUIDAsString(accountPair.first)));
                rec.push_back(Pair("label", accountPair.second->getLabel()));
                rec.push_back(Pair("state", GetAccountStateString(accountPair.second->m_State)));
                rec.push_back(Pair("type", GetAccountTypeString(accountPair.second->m_Type)));
                rec.push_back(Pair("HD_type", "legacy"));
                allAccounts.push_back(rec);
            }
            continue;
        }
        if (accountPair.second->m_State == AccountState::Shadow && !(sStateSearch == "Shadow"||sStateSearch == "ShadowChild"))
            continue;
        if (forSeed && ((CAccountHD*)accountPair.second)->getSeedUUID() != forSeed->getUUID())
            continue;

        UniValue rec(UniValue::VOBJ);
        rec.push_back(Pair("UUID", getUUIDAsString(accountPair.first)));
        rec.push_back(Pair("label", accountPair.second->getLabel()));
        rec.push_back(Pair("state", GetAccountStateString(accountPair.second->m_State)));
        rec.push_back(Pair("type", GetAccountTypeString(accountPair.second->m_Type)));
        rec.push_back(Pair("HD_type", "HD"));
        rec.push_back(Pair("HDindex", (uint64_t) dynamic_cast<CAccountHD*>(accountPair.second)->getIndex()));

        allAccounts.push_back(rec);
    }

    return allAccounts;
}

static UniValue getmnemonicfromseed(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getmnemonicfromseed \"seed\" \n"
            "\nGet the mnemonic of a HD seed.\n"
            "1. \"seed\"        (string, required) The unique UUID for the seed that we want mnemonics of, or \"\" for the active seed.\n"
            "\nResult:\n"
            "\nReturn the mnemonic as a string.\n"
            "\nNote it is important to ensure that nobody gets access to this mnemonic or all funds in accounts made from the seed can be compromised.\n"
            "\nExamples:\n"
            + HelpExampleCli("getmnemonicfromseed \"827f0000-0300-0000-0000-000000000000\"", "")
            + HelpExampleRpc("getmnemonicfromseed \"827f0000-0300-0000-0000-000000000000\"", ""));


    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CHDSeed* seed = SeedFromValue(pwallet, request.params[0], true);

    EnsureWalletIsUnlocked(pwallet);

    return seed->getMnemonic().c_str();
}

static UniValue getreadonlyseed(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getreadonlyseed \"seed\" \n"
            "\nGet the public key of an HD seed, this can be used to import the seed as a read only seed in another wallet.\n"
            "1. \"seed\"        (string, required) The unique UUID for the seed that we want the public key of, or \"\" for the active seed.\n"
            "\nNote the seed must be a 'non hardened' BIP44NH seed and not a regular seed.\n"
            "\nResult:\n"
            "\nReturn the public key as a string.\n"
            "\nNote it is important to be careful with and protect access to this public key as if it is compromised it can weaken security in cases where private keys are also compromised.\n"
            "\nExamples:\n"
            + HelpExampleCli("getreadonlyseed \"827f0000-0300-0000-0000-000000000000\"", "")
            + HelpExampleRpc("getreadonlyseed \"827f0000-0300-0000-0000-000000000000\"", ""));

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CHDSeed* seed = SeedFromValue(pwallet, request.params[0], true);

    if (seed->m_type != CHDSeed::SeedType::BIP44NoHardening)
        throw std::runtime_error("Can only use command with a non-hardened BIP44 seed");

    EnsureWalletIsUnlocked(pwallet);

    return seed->getPubkey().c_str();
}

static UniValue listseeds(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "listseeds \n"
            "\nReturn the UUID for all wallet seeds.\n"
            "\nExamples:\n"
            + HelpExampleCli("listseeds", "")
            + HelpExampleRpc("listseeds", ""));

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    UniValue AllSeeds(UniValue::VARR);

    for (const auto& seedPair : pwallet->mapSeeds)
    {
        UniValue rec(UniValue::VOBJ);
        rec.push_back(Pair("UUID", getUUIDAsString(seedPair.first)));
        rec.push_back(Pair("type", StringFromSeedType(seedPair.second)));
        if (seedPair.second->IsReadOnly())
            rec.push_back(Pair("readonly", "true"));
        AllSeeds.push_back(rec);
    }

    return AllSeeds;
}

static UniValue rotatewitnessaddress(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "rotatewitnessaddress \"funding_account\" \"witness_address\" \n"
            "\nChange the \"witnessing key\" of a witness account, the wallet needs to be unlocked to do this, the \"spending key\" will remain unchanged. \n"
            "1. \"funding_account\"  (string, required) The unique UUID or label for the account from which money will be removed to pay for the transaction fee.\n"
            "2. \"witness_address\"  (string, required) The Gulden address for the witness key.\n"
            "\nResult:\n"
            "[\n"
            "     \"txid\":\"txid\",   (string) The txid of the created transaction\n"
            "     \"fee_amount\":n   (number) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("rotatewitnessaddress \"My account\" 2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN", "")
            + HelpExampleRpc("rotatewitnessaddress \"My account\" 2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' address.
    CGuldenAddress witnessAddress(request.params[1].get_str());
    bool isValid = witnessAddress.IsValidWitness(Params());

    if (!isValid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Not a valid witness address [%s].", request.params[1].get_str()));

    const auto& unspentWitnessOutputs = getCurrentOutputsForWitnessAddress(witnessAddress);
    if (unspentWitnessOutputs.size() == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Address does not contain any witness outputs [%s].", request.params[1].get_str()));

    try {
        std::string txid;
        CAmount fee;
        rotatewitnessaddresshelper(fundingAccount, unspentWitnessOutputs, pwallet, &txid, &fee);
        UniValue result(UniValue::VOBJ);
        result.push_back(Pair("txid", txid));
        result.push_back(Pair("fee_amount", ValueFromAmount(fee)));
        return result;
    }
    catch (witness_error& e) {
        throw JSONRPCError(e.code(), e.what());
    }
    catch (std::runtime_error& e) {
        throw JSONRPCError(RPC_MISC_ERROR, e.what());
    }
}

static UniValue verifywitnessaddress(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "verifywitnessaddress \"witness_address\" \n"
            "\nVerify that a witness address is in good working order. Wallet must have both public keys, witness key must be available to sign in an unencrypted form so that wallet can witness while locked.\n"
            "1. \"witness_address\"  (string, required) The Gulden address for the witness key.\n"
            "\nResult:\n"
            "[\n"
            "     \"validity\",   (boolean) True if all keys are present and in correct form, false otherwise.\n"
            "     \"info\",       (string)  Information on the error.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("rotatewitnessaddress \"My account\" 2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN", "")
            + HelpExampleRpc("rotatewitnessaddress \"My account\" 2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    UniValue result(UniValue::VOBJ);
    // NB! Wallet should be locked for this test
    if (!pwallet->IsCrypted())
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "verifywitnessaddress can only be used on encrypted wallets"));
        return result;
    }
    if (!pwallet->IsLocked())
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "verifywitnessaddress can only be used on locked wallets"));
        return result;
    }

    // arg1 - 'to' address.
    CGuldenAddress witnessAddress(request.params[1].get_str());
    bool isValid = witnessAddress.IsValidWitness(Params());

    if (!isValid)
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Not a valid witness address"));
        return result;
    }

    const auto& unspentWitnessOutputs = getCurrentOutputsForWitnessAddress(witnessAddress);
    if (unspentWitnessOutputs.size() == 0)
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Not an active witness address"));
        return result;
    }

    if (unspentWitnessOutputs.size() > 1)
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Too many outputs for account so unsure which one to validate, aborting."));
        return result;
    }

    // Find the account
    const auto& [currentWitnessTxOut, currentWitnessHeight, currentWitnessOutpoint] = unspentWitnessOutputs[0];
    (unused) currentWitnessHeight;
    (unused) currentWitnessOutpoint;
    CAccount* witnessAccount = pwallet->FindAccountForTransaction(currentWitnessTxOut);
    if (!witnessAccount)
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Unable to determine account for witness address."));
        return result;
    }

    // Get the current witness details
    CTxOutPoW2Witness currentWitnessDetails;
    GetPow2WitnessOutput(currentWitnessTxOut, currentWitnessDetails);

    CPubKey pubKey;
    if (!witnessAccount->GetPubKey(currentWitnessDetails.witnessKeyID, pubKey))
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Unable to retrieve public witness key"));
        return result;
    }
    if (!witnessAccount->GetPubKey(currentWitnessDetails.spendingKeyID, pubKey))
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Unable to retrieve public spending key"));
        return result;
    }

    CKey privKey;
    if (!witnessAccount->GetKey(currentWitnessDetails.witnessKeyID, privKey))
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Able to retrieve witness signing key; this should not be possible as key should be encrypted."));
        return result;
    }
    if (witnessAccount->GetKey(currentWitnessDetails.spendingKeyID, privKey))
    {
        result.push_back(Pair("validity", false));
        result.push_back(Pair("info", "Unable to retrieve spending signing key; key may be incorrectly encrypted."));
        return result;
    }

    result.push_back(Pair("validity", true));
    result.push_back(Pair("info", ""));
    return result;
}


static UniValue checkpointinvalidate(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "checkpointinvalidate \"block_hash\"\n"
            "\nPermanently marks a block as invalid, as if it violated a consensus rule.\n"
            "\nArguments:\n"
            "1. \"block_hash\"   (string, required) the hash of the block to mark as invalid\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("invalidateblock", "\"block_hash\"")
            + HelpExampleRpc("invalidateblock", "\"block_hash\"")
        );

    std::string strHash = request.params[0].get_str();
    uint256 hash(uint256S(strHash));

    if (!Checkpoints::SendCheckpointInvalidate(hash, Params()))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Failed to send invalidate-checkpoint");
    }

    return NullUniValue;
}

static UniValue rotatewitnessaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "rotatewitnessaccount \"funding_account\" \"witness_account\" \n"
            "\nChange the \"witnessing key\" of a witness account, the wallet needs to be unlocked to do this, the \"spending key\" will remain unchanged. \n"
            "1. \"funding_account\"  (string, required) The unique UUID or label for the account from which money will be removed to pay for the transaction fee.\n"
            "2. \"witness_account\"  (string, required) The unique UUID or label for the witness account that will hold the locked funds.\n"
            "\nResult:\n"
            "[\n"
            "     \"txid\":\"txid\",   (string) The txid of the created transaction\n"
            "     \"fee_amount\":n   (number) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("rotatewitnessaddress \"My account\" \"My witness account\"", "")
            + HelpExampleRpc("rotatewitnessaddress \"My account\" \"My witness account\"", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' account.
    CAccount* witnessAccount = AccountFromValue(pwallet, request.params[1], false);
    if (!witnessAccount)
        throw std::runtime_error(strprintf("Unable to locate witness account [%s].",  request.params[1].get_str()));

    try {
        std::string txid;
        CAmount fee;
        rotatewitnessaccount(pwallet, fundingAccount, witnessAccount, &txid, &fee);
        UniValue result(UniValue::VOBJ);
        result.push_back(Pair("txid", txid));
        result.push_back(Pair("fee_amount", ValueFromAmount(fee)));
        return result;
    }
    catch (witness_error& e) {
        throw JSONRPCError(e.code(), e.what());
    }
    catch (std::runtime_error& e) {
        throw JSONRPCError(RPC_MISC_ERROR, e.what());
    }

}

static UniValue renewwitnessaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "renewwitnessaccount \"witness_account\" \n"
            "\nRenew an expired witness account. \n"
            "1. \"funding_account\"        (required) The unique UUID or label for the account.\n"
            "2. \"witness_account\"        (required) The unique UUID or label for the account.\n"
            "\nResult:\n"
            "[\n"
            "     \"txid\",                (string) The txid of the created transaction\n"
            "     \"fee_amount\"           (number) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("renewwitnessaccount \"My account\" \"My witness account\"", "")
            + HelpExampleRpc("renewwitnessaccount \"My account\" \"My witness account\"", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' account.
    CAccount* witnessAccount = AccountFromValue(pwallet, request.params[1], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    if ((!witnessAccount->IsPoW2Witness()) || witnessAccount->IsFixedKeyPool())
    {
        throw JSONRPCError(RPC_MISC_ERROR, "Cannot split a witness-only account as spend key is required to do this.");
    }

    //fixme: (PHASE5) - Share common code with GUI::requestRenewWitness
    std::string strError;
    CMutableTransaction tx(CURRENT_TX_VERSION_POW2);
    CReserveKeyOrScript changeReserveKey(pactiveWallet, fundingAccount, KEYCHAIN_EXTERNAL);
    CAmount transactionFee;
    if (!pactiveWallet->PrepareRenewWitnessAccountTransaction(fundingAccount, witnessAccount, changeReserveKey, tx, transactionFee, strError))
    {
        throw std::runtime_error(strprintf("Failed to create renew transaction [%s]", strError.c_str()));
    }

    uint256 finalTransactionHash;
    {
        LOCK2(cs_main, pactiveWallet->cs_wallet);
        if (!pactiveWallet->SignAndSubmitTransaction(changeReserveKey, tx, strError, &finalTransactionHash))
        {
            throw std::runtime_error(strprintf("Failed to sign renew transaction [%s]", strError.c_str()));
        }
    }

    // Clear the failed flag in UI, and remove the 'renew' button for immediate user feedback.
    witnessAccount->SetWarningState(AccountStatus::WitnessPending);
    static_cast<const CGuldenWallet*>(pactiveWallet)->NotifyAccountWarningChanged(pactiveWallet, witnessAccount);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair(finalTransactionHash.GetHex(), ValueFromAmount(transactionFee)));
    return result;
}

static UniValue splitwitnessaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 3)
        throw std::runtime_error(
            "splitwitnessaccount \"funding_account\" \"witness_account\" \"amounts\" \n"
            "\nSplit a witness address into two seperate witness addresses, all details of the addresses remain identical other than a reduction in amounts.\n"
            "\nThis is useful in the event that an account has exceeded 1 percent of the network weight. \n"
            "1. \"funding_account\"        (required) The unique UUID or label for the account.\n"
            "2. \"witness_account\"        (required) The unique UUID or label for the account.\n"
            "3. \"amounts\"                (string, required) A json object with amounts for the new addresses\n"
            "    {\n"
            "      \"amount\"              (numeric) The amount that should go in each new account\n"
            "      ,...\n"
            "    }\n"
            "\nResult:\n"
            "[\n"
            "     \"txid\",                (string) The txid of the created transaction\n"
            "     \"fee_amount\"           (number) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("splitwitnessaccount \"My account\" \"My witness account\"  [10000, 5000, 5000]", "")
            + HelpExampleRpc("splitwitnessaccount \"My account\" \"My witness account\"  [10000, 5000, 5000]", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' account.
    CAccount* witnessAccount = AccountFromValue(pwallet, request.params[1], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[1].get_str()));

    if ((!witnessAccount->IsPoW2Witness()) || witnessAccount->IsFixedKeyPool())
    {
        throw JSONRPCError(RPC_MISC_ERROR, "Cannot split a witness-only account as spend key is required to do this.");
    }

    // arg3 - 'split' paramaters
    std::vector<UniValue> splitInto = request.params[2].getValues();
    if (splitInto.size() < 2)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Split command requires at least two outputs");

    std::vector<CAmount> splitAmounts;
    for (const auto& unparsedSplitAmount : splitInto)
    {
        CAmount splitValue = AmountFromValue(unparsedSplitAmount);
        splitAmounts.emplace_back(splitValue);
    }

    try {
        std::string txid;
        CAmount fee;
        redistributewitnessaccount(pwallet, fundingAccount, witnessAccount, splitAmounts, &txid, &fee);
        UniValue result(UniValue::VOBJ);
        result.push_back(Pair("txid", txid));
        result.push_back(Pair("fee_amount", ValueFromAmount(fee)));
        return result;
    }
    catch (witness_error& e) {
        throw JSONRPCError(e.code(), e.what());
    }
    catch (std::runtime_error& e) {
        throw JSONRPCError(RPC_MISC_ERROR, e.what());
    }
}

static UniValue mergewitnessaccount(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "mergewitnessaccount \"funding_account\" \"witness_account\"\n"
            "\nMerge multiple witness addresses into a single one.\n"
            "\nAddresses must share identical characteristics other than \"amount\" and therefore this will usually only work on addresses that were created via \"splitwitnessaccount\".\n"
            "\nThis is useful in the event that the network weight has risen significantly and an account that was previously split could now earn better as a single account. \n"
            "1. \"funding_account\"        (required) The unique UUID or label for the account.\n"
            "2. \"witness_account\"        (required) The unique UUID or label for the account.\n"
            "\nResult:\n"
            "[\n"
            "     \"txid\",                (string) The txid of the created transaction\n"
            "     \"fee_amount\"           (number) The fee that was paid.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("mergewitnessaccount \"My account\" \"My witness account\"", "")
            + HelpExampleRpc("mergewitnessaccount \"My account\" \"My witness account\"", ""));

    // Basic sanity checks.
    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");
    if (!IsSegSigEnabled(chainActive.TipPrev()))
        throw std::runtime_error("Cannot use this command before segsig activates");

    EnsureWalletIsUnlocked(pwallet);

    // arg1 - 'from' account.
    CAccount* fundingAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    // arg2 - 'to' account.
    CAccount* witnessAccount = AccountFromValue(pwallet, request.params[1], false);
    if (!fundingAccount)
        throw std::runtime_error(strprintf("Unable to locate funding account [%s].",  request.params[0].get_str()));

    if ((!witnessAccount->IsPoW2Witness()) || witnessAccount->IsFixedKeyPool())
    {
        throw JSONRPCError(RPC_MISC_ERROR, "Cannot split a witness-only account as spend key is required to do this.");
    }

    const auto& unspentWitnessOutputs = getCurrentOutputsForWitnessAccount(witnessAccount);
    if (unspentWitnessOutputs.size() == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Account does not contain any witness outputs [%s].", request.params[1].get_str()));

    if (unspentWitnessOutputs.size() == 1)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, strprintf("Account only contains one witness output at least two are required to merge [%s].", request.params[1].get_str()));

    CAmount totalAmount = std::accumulate(unspentWitnessOutputs.begin(), unspentWitnessOutputs.end(), CAmount(0), [](const CAmount acc, const auto& it){
        const CTxOut& txOut = std::get<0>(it);
        return acc + txOut.nValue;
    });

    try {
        std::string txid;
        CAmount fee;
        redistributewitnessaccount(pwallet, fundingAccount, witnessAccount, std::vector({totalAmount}), &txid, &fee);
        UniValue result(UniValue::VOBJ);
        result.push_back(Pair("txid", txid));
        result.push_back(Pair("fee_amount", ValueFromAmount(fee)));
        return result;
    }
    catch (witness_error& e) {
        throw JSONRPCError(e.code(), e.what());
    }
    catch (std::runtime_error& e) {
        throw JSONRPCError(RPC_MISC_ERROR, e.what());
    }
}

static UniValue setwitnesscompound(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "setwitnesscompound \"witness_account\" \"amount\"\n"
            "\nSet whether a witness account should compound or not.\n"
            "\nCompounding is controlled as follows:\n"
            "\n    1) When set to 0 no compounding will be done, all rewards will be sent to the non-compound output set by \"setwitnessgeneration\" or a key in the account if \"setwitnessgeneration\" has not been called.\n"
            "\n    2) When set to a positive number \"n\", earnings up until \"n\" will be compounded, and the remainder will be sent to the non-compound output (as describe in 1).\n"
            "\n    3) When set to a negative number \"n\", \"n\" will be deducted and sent to a non-compound output (as described in 1) and the remainder will be compounded.\n"
            "\nIn all cases it is important to remember the following:\n"
            "\n    4) Transaction fees and not just the witness reward can be compounded, so while the witness reward is 20 NLG compounding amount should be set considering possible transaction fees as well.\n"
            "\n    5) A maximum of 40 NLG can be compounded, regardless of whether a block contains more fees. In the event that there are additional fees to distribute after applying the compounding settings, the settings will be ignored for the additional fees and paid to a non-compound output (as described in 1)\n"
            "\nArguments:\n"
            "1. \"witness_account\"            (string) The UUID or unique label of the account.\n"
            "2. amount                        (numeric or string, required) The amount in " + CURRENCY_UNIT + "\n"
            "\nResult:\n"
            "[\n"
            "     \"account_uuid\",            (string) The UUID of the account that has been modified.\n"
            "     \"amount\"                   (string) The amount that has been set.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("setwitnesscompound \"My witness account\" 20", "")
            + HelpExampleRpc("setwitnesscompound \"My witness account\" 20", ""));

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CAccount* forAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!forAccount)
        throw std::runtime_error("Invalid account name or UUID");

    if (!forAccount->IsPoW2Witness())
        throw std::runtime_error(strprintf("Specified account is not a witness account [%s].",  request.params[0].get_str()));

    CAmount amount = AmountFromValue(request.params[1], true);

    CWalletDB walletdb(*pwallet->dbw);
    forAccount->setCompounding(amount, &walletdb);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair(getUUIDAsString(forAccount->getUUID()), ValueFromAmount(forAccount->getCompounding())));
    return result;
}

static UniValue getwitnesscompound(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getwitnesscompound \"witness_account\"\n"
            "\nGet the current compound setting for an account.\n"
            "\nArguments:\n"
            "1. \"witness_account\"        (string) The UUID or unique label of the account.\n"
            "\nResult:\n"
            "\nReturn the current amount set for the account.\n"
            "\nCompounding is controlled as follows:\n"
            "\n    1) When set to 0 no compounding will be done, all rewards will be sent to the non-compound output set by \"setwitnessgeneration\" or a key in the account if \"setwitnessgeneration\" has not been called.\n"
            "\n    2) When set to a positive number \"n\", earnings up until \"n\" will be compounded, and the remainder will be sent to the non-compound output (as describe in 1).\n"
            "\n    3) When set to a negative number \"n\", \"n\" will be deducted and sent to a non-compound output (as described in 1) and the remainder will be compounded.\n"
            "\nIn all cases it is important to remember the following:\n"
            "\n    4) Transaction fees and not just the witness reward can be compounded, so while the witness reward is 20 NLG compounding amount should be set considering possible transaction fees as well.\n"
            "\n    5) A maximum of 40 NLG can be compounded, regardless of whether a block contains more fees. In the event that there are additional fees to distribute after applying the compounding settings, the settings will be ignored for the additional fees and paid to a non-compound output (as described in 1)\n"
            "\nExamples:\n"
            + HelpExampleCli("getwitnesscompound \"My witness account\"", "")
            + HelpExampleRpc("getwitnesscompound \"My witness account\"", ""));

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    CAccount* forAccount = AccountFromValue(pwallet, request.params[0], false);
    if (!forAccount)
        throw std::runtime_error("Invalid account name or UUID");

    if (!forAccount->IsPoW2Witness())
        throw std::runtime_error(strprintf("Specified account is not a witness account [%s].",  request.params[0].get_str()));

    return ValueFromAmount(forAccount->getCompounding());
}

static UniValue setwitnessrewardscript(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 3 )
        throw std::runtime_error(
            "setwitnessrewardscript \"witness_account\" \"destination\" force_pubkey \n"
            "\nSet the output key into which all non-compound witness earnings will be paid.\n"
            "\nSee \"setwitnesscompound\" for how to control compounding and additional information.\n"
            "1. \"witness_account\"        (required) The unique UUID or label for the account.\n"
            "2. \"destination\"           (required) An address or hex encoded script or public key. Set empty string to reset the reward script.\n"
            "3. force_pubkey              (boolean, optional, default=false) Cause command to fail if an invalid pubkey is passed, without this the pubkey may be imported as a script.\n"
            "\nResult:\n"
            "[\n"
            "     \"account_uuid\",        (string) The UUID of the account that has been modified.\n"
            "     \"amount\"               (string) The amount that has been set.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("setwitnessrewardscript \"my witness account\" \"Vb5YMjiTA9BUYi9zPToKg3wAAdrpHNp1V2hSBVHpgLMm9sPojhnX\"", "")
            + HelpExampleRpc("setwitnessrewardscript \"my witness account\" \"Vb5YMjiTA9BUYi9zPToKg3wAAdrpHNp1V2hSBVHpgLMm9sPojhnX\"", ""));

    CAccount* forAccount = AccountFromValue(pwallet, request.params[0], false);

    if (!forAccount)
        throw std::runtime_error("Invalid account name or UUID");

    if (!forAccount->IsPoW2Witness())
        throw std::runtime_error(strprintf("Specified account is not a witness account [%s].",  request.params[0].get_str()));


    bool forcePubKey = false;
    if (request.params.size() > 2)
        forcePubKey = request.params[2].get_bool();

    std::string pubKeyOrScript = request.params[1].get_str();

    CScript scriptForNonCompoundPayments;

    CGuldenAddress address(pubKeyOrScript);
    if (address.IsValid()) {
        scriptForNonCompoundPayments = GetScriptForDestination(address.Get());
    }
    else if (!pubKeyOrScript.empty()) {
        if (!IsHex(pubKeyOrScript))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Data is neither hex encoded nor a valid address");

        // Try public key first.
        std::vector<unsigned char> data(ParseHex(pubKeyOrScript));
        CPubKey pubKey(data.begin(), data.end());
        if (pubKey.IsFullyValid())
        {
            scriptForNonCompoundPayments = CScript() << ToByteVector(pubKey) << OP_CHECKSIG;
        }
        else
        {
            if (forcePubKey)
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Not a valid hex encoded public key");

            // Not a public key so treat it as a script.
            scriptForNonCompoundPayments = CScript(data.begin(), data.end());
            if (!scriptForNonCompoundPayments.HasValidOps())
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Data is hex encoded, but not a valid pubkey or script");
            }
        }
    }

    CWalletDB walletdb(*pwallet->dbw);
    forAccount->setNonCompoundRewardScript(scriptForNonCompoundPayments, &walletdb);

    witnessScriptsAreDirty = true;

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair(getUUIDAsString(forAccount->getUUID()), HexStr(forAccount->getNonCompoundRewardScript())));
    return result;
}

static UniValue getwitnessrewardscript(const JSONRPCRequest& request)
{
     #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getwitnessrewardscript \"witness_account\" \n"
            "\nGet the output key into which all non-compound witness earnings will be paid.\n"
            "\nSee \"getwitnesscompound\" for how to control compounding and additional information.\n"
            "1. \"witness_account\"    (required) The unique UUID or label for the account.\n"
            "\nResult:\n"
            "[\n"
            "     \"account_uuid\",    (string) The UUID of the account that has been modified.\n"
            "     \"hex_script\"       (string) Hex encoded script that has been set.\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("setwitnessrewardscript \"my witness account\"", "")
            + HelpExampleRpc("setwitnessrewardscript \"my witness account\"", ""));

    CAccount* forAccount = AccountFromValue(pwallet, request.params[0], false);

    if (!forAccount)
        throw std::runtime_error("Invalid account name or UUID");

    if (!forAccount->IsPoW2Witness())
        throw std::runtime_error(strprintf("Specified account is not a witness account [%s].",  request.params[0].get_str()));

    UniValue result(UniValue::VOBJ);
    if (!forAccount->hasNonCompoundRewardScript())
    {
        result.push_back(Pair(getUUIDAsString(forAccount->getUUID()), ""));
    }
    else
    {
        result.push_back(Pair(getUUIDAsString(forAccount->getUUID()), HexStr(forAccount->getNonCompoundRewardScript())));
    }
    return result;
}

static UniValue setwitnessrewardtemplate(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "setwitnessrewardtemplate \"witness_account\" [[\"destination1\" (,\"amount\") (,\"percentage%\") (,\"remainder\") (,\"compound_overflow\")], [\"destination2\" ...], ...]\n"
            "\nSet the template to control where witness earnings are paid. Multiple destianations can be specified, compounding or not each receving a fixed\n"
            "amount and/or a percentage of the witness amounts earned.\n"
            "1. \"witness_account\"        (required) The unique UUID or label for the account.\n"
            "2. \"destinationX\"           (required) an address or one of the special keywords: \"account\" or \"compound\"\n"
            "                                  where: \"compound\", compound into witness\n"
            "                                         \"account\", to witness account, but not compounding (ie. spendable)\n"
            "3. amount                     (string, optional) Fixed amount for this destination.\n"
            "4. percentage                 (string, optional) Percentage of remaining non-fixed amount for this destination (postfixed with % symbol).\n"
            "4. remainder                  (string, optional) The remainder marked destination receives amount reamining after dishing out fixed and percentage amounts.\n"
            "5. compound_overflow          (string, optional) The compound_overflow marked destination receives any excess compound.\n"
            "\nResult:\n"
            "(string) The UUID of the account that has been modified.\n"
            "\nRemarks:\n"
            "If a reward template is set on the account it overrides the witness-compound setting (see \"setwitnesscompound\").\n"
            "For each entry the destination has to be the first element. The order of the following elements is arbitrary. When multiple entries have remainder, compound or compound_overflow set "
            "behaviour is undefined (only one can receive the remainder). When using percentages it is recommended to also use a remainder, instead of giving multiple "
            "entries that total 100%. Because percentages are floating point specifying a total of 100% can lead to rounding errors and over specification.\n"
            "If a reward script is set (see \"setwitnessrewardscript\") that is used as the \"account\" destination.\n"
            "When resulting compound exceeds the allowed amount without a \"compound_overflow\" in the template, the overflow will go to the remainder (which cannot be on the \"compound\" destination in this case).\n"
            "\nDistribution of rewards:\n"
            "1. All fixed amounts are distributed (including compound)\n"
            "2. Percentages of the remaining amount are dihsed out.\n"
            "3. Any remaining amount goes to the remainder destination.\n"
            "4. If the compound amount resulting from above calculation exceeds the maximum allowed the maximum will be compound and the excess amount will go to compound_overflow.\n"
            "\nExamples:\n"
            "Assuming there is 90 NLG witness reward to dive, then in the example below the fixed amount to divide is 40 (10 + 30), leaving 50 for percentage splits. Only 40% (20 NLG) is specified (5% + 5% + 30%), leaving 30 NLG as remainder.\n"
            "So the distribution is: 2.5 NLG to TRVQzTaFGt1cQcDdgAJGwnzFfFgUbR1PnF, 40 non-compounding to the witness account, 2.5 NLG to TBb5KJ3jnq7Xk5uwWV7dAyRmSEgfvszevo (compound overflow = 0) and 45 is compounded into the witness.\n"
            + HelpExampleCli("setwitnessrewardtemplate \"my witness account\" '[[\"TRVQzTaFGt1cQcDdgAJGwnzFfFgUbR1PnF\", \"5%\"], [\"account\", \"10\", \"remainder\"],[\"TBb5KJ3jnq7Xk5uwWV7dAyRmSEgfvszevo\", \"5%\", \"compound_overflow\"], [\"compound\", \"30\", \"30%\"]]'", "")
            + HelpExampleRpc("setwitnessrewardtemplate \"my witness account\" '[[\"TRVQzTaFGt1cQcDdgAJGwnzFfFgUbR1PnF\", \"5%\"], [\"account\", \"10\", \"remainder\"],[\"TBb5KJ3jnq7Xk5uwWV7dAyRmSEgfvszevo\", \"5%\", \"compound_overflow\"], [\"compound\", \"30\", \"30%\"]]'", ""));

    CAccount* forAccount = AccountFromValue(pwallet, request.params[0], false);

    if (!forAccount)
        throw std::runtime_error("Invalid account name or UUID");

    if (!forAccount->IsPoW2Witness())
        throw std::runtime_error(strprintf("Specified account is not a witness account [%s].",  request.params[0].get_str()));

    const std::vector<UniValue>& destinations = request.params[1].get_array().getValues();

    CWitnessRewardTemplate rewardTemplate;

    for (const UniValue& dst: destinations) {
        const std::vector<UniValue>& dstArr = dst.get_array().getValues();
        if (dstArr.size() < 2)
            throw JSONRPCError(RPC_INVALID_PARAMS, "Need destination and at least one quantity specifier");

        CWitnessRewardDestination rewardDestination;

        std::string destSpec = dstArr[0].getValStr();
        if (destSpec == "compound") {
            rewardDestination.type = CWitnessRewardDestination::DestType::Compound;
        }
        else if (destSpec == "account") {
            rewardDestination.type = CWitnessRewardDestination::DestType::Account;
        }
        else {
            rewardDestination.type = CWitnessRewardDestination::DestType::Address;
            rewardDestination.address = CGuldenAddress(destSpec);
        }

        for (auto it = ++dstArr.begin(); it != dstArr.end(); it++) {
            std::string qtySpec = it->getValStr();
            if (qtySpec == "remainder") {
                rewardDestination.takesRemainder = true;
            }
            else if (qtySpec == "compound_overflow") {
                rewardDestination.takesCompoundOverflow = true;
            }
            else if (qtySpec.size() > 0 && qtySpec[qtySpec.size() - 1] == '%') {
                rewardDestination.percent = std::stod(qtySpec.substr(0, qtySpec.size() - 1)) / 100.0;
            }
            else {
                rewardDestination.amount = AmountFromValue(*it);
            }
        }

        rewardTemplate.destinations.push_back(rewardDestination);
    }

    rewardTemplate.validate(GetBlockSubsidyWitness(chainActive.Height()));

    CWalletDB walletdb(*pwallet->dbw);
    forAccount->setRewardTemplate(rewardTemplate, &walletdb);

    UniValue result(getUUIDAsString(forAccount->getUUID()));
    return result;
}

static UniValue getwitnessrewardtemplate(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getwitnessrewardtemplate \"witness_account\" \n"
            "\nGet the template how witness earnings will be paid..\n"
            "\nSee \"setwitnessrewardtemplate\" for additional information.\n"
            "1. \"witness_account\"    (required) The unique UUID or label for the account.\n"
            "\nExamples:\n"
            + HelpExampleCli("getwitnessrewardtemplate \"my witness account\"", "")
            + HelpExampleRpc("getwitnessrewardtemplate \"my witness account\"", ""));

    CAccount* forAccount = AccountFromValue(pwallet, request.params[0], false);

    if (!forAccount)
        throw std::runtime_error("Invalid account name or UUID");

    if (!forAccount->IsPoW2Witness())
        throw std::runtime_error(strprintf("Specified account is not a witness account [%s].",  request.params[0].get_str()));

    UniValue result(UniValue::VOBJ);
    if (!forAccount->hasRewardTemplate())
    {
        result.push_back(Pair(getUUIDAsString(forAccount->getUUID()), ""));
    }
    else
    {
        CWitnessRewardTemplate rewardTemplate = forAccount->getRewardTemplate();
        UniValue templateArray(UniValue::VARR);
        for (const CWitnessRewardDestination& dest: rewardTemplate.destinations) {
            UniValue destArray(UniValue::VARR);
            std::string destStr;

            switch (dest.type) {
            case CWitnessRewardDestination::DestType::Address:
                destStr = dest.address.ToString();
                break;
            case CWitnessRewardDestination::DestType::Compound:
                destStr = "compound";
                break;
            case CWitnessRewardDestination::DestType::Account:
                destStr = "account";
                break;
            }
            destArray.push_back(destStr);

            if (dest.amount > 0)
                destArray.push_back(ValueFromAmount(dest.amount));

            if (dest.percent > 0.0)
                destArray.push_back(strprintf("%.2f%%", 100.0 * dest.percent));

            if (dest.takesRemainder)
                destArray.push_back("remainder");

            if (dest.takesCompoundOverflow)
                destArray.push_back("compound_overflow");

            templateArray.push_back(destArray);
        }

        result.push_back(Pair(getUUIDAsString(forAccount->getUUID()), templateArray));
    }
    return result;
}

static UniValue getwitnessaccountkeys(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getwitnessaccountkeys \"witness_account\" \n"
            "\nGet the witness keys of an HD account, this can be used to import the account as a witness only account in another wallet via the \"importwitnesskeys\" command.\n"
            "\nA single account can theoretically contain multiple keys, if it has been split \"splitwitnessaccount\", this will include all of them \n"
            "1. \"witness_account\"        (required) The unique UUID or label for the account.\n"
            "\nResult:\n"
            "\nReturn the private witness keys as an encoded string, that can be used with the \"importwitnesskeys\" command.\n"
            "\nNB! The exported private key is only the \"witnessing\" key and not the \"spending\" key for the witness account.\n"
            "\nIf the \"witness\" key is compromised your funds will remain completely safe however the attacker will be able to use the key to claim your earnings.\n"
            "\nIf you believe your key is or may have been compromised use \"rotatewitnessaccount\" to rotate to a new witness key.\n"
            "\nExamples:\n"
            + HelpExampleCli("getwitnessaccountkeys \"My witness account\"", "")
            + HelpExampleRpc("getwitnessaccountkeys \"My witness account\"", ""));

    CAccount* forAccount = AccountFromValue(pwallet, request.params[0], false);

    if (!forAccount)
        throw std::runtime_error("Invalid account name or UUID");

    if (!forAccount->IsPoW2Witness())
        throw std::runtime_error("Can only be used on a witness account.");

    if (!chainActive.Tip())
        throw std::runtime_error("Wait for chain to synchronise before using command.");

    if (!IsPow2WitnessingActive(chainActive.Tip(), Params(), chainActive, nullptr))
        throw std::runtime_error("Wait for witnessing to activate before using this command.");

    EnsureWalletIsUnlocked(pwallet);

    std::map<COutPoint, Coin> allWitnessCoins;
    if (!getAllUnspentWitnessCoins(chainActive, Params(), chainActive.Tip(), allWitnessCoins))
        throw std::runtime_error("Failed to enumerate all witness coins.");

    std::string linkUrl = witnessKeysLinkUrlForAccount(pwallet, forAccount);

    if (linkUrl.empty())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Witness account has no active keys.");

    return linkUrl;
}

static UniValue getwitnessaddresskeys(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getwitnessaddresskeys \"witness_address\" \n"
            "\nGet the witness key of an HD address, this can be used to import the account as a witness only account in another wallet via the \"importwitnesskeys\" command.\n"
            "1. \"witness_address\"        (required) The Gulden address for the witness key.\n"
            "\nResult:\n"
            "\nReturn the private witness key as an encoded string, that can be used with the \"importwitnesskeys\" command.\n"
            "\nNB! The exported private key is only the \"witnessing\" key and not the \"spending\" key for the witness account.\n"
            "\nIf the \"witness\" key is compromised your funds will remain completely safe however the attacker will be able to use the key to claim your earnings.\n"
            "\nIf you believe your key is or may have been compromised use \"rotatewitnessaccount\" to rotate to a new witness key.\n"
            "\nExamples:\n"
            + HelpExampleCli("getwitnessaddresskeys \"2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN\"", "")
            + HelpExampleRpc("getwitnessaddresskeys \"2ZnFwkJyYeEftAoQDe7PC96t2Y7XMmKdNtekRdtx32GNQRJztULieFRFwQoQqN\"", ""));

    CGuldenAddress forAddress(request.params[0].get_str());
    bool isValid = forAddress.IsValidWitness(Params());

    if (!isValid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Not a valid witness address.");

    EnsureWalletIsUnlocked(pwallet);

    std::string witnessAccountKeys = "";
    for (const auto& [accountUUID, forAccount] : pwallet->mapAccounts)
    {
        (unused)accountUUID;
        CPoW2WitnessDestination dest = boost::get<CPoW2WitnessDestination>(forAddress.Get());
        if (forAccount->HaveKey(dest.witnessKey))
        {
            CKey witnessPrivKey;
            if (!forAccount->GetKey(dest.witnessKey, witnessPrivKey))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Unable to retrieve key for witness address.");
            }
            //fixme: (PHASE4) - to be 100% correct we should export the creation time of the actual key (where available) and not getEarliestPossibleCreationTime - however getEarliestPossibleCreationTime will do for now.
            witnessAccountKeys += CGuldenSecret(witnessPrivKey).ToString() + strprintf("#%s", forAccount->getEarliestPossibleCreationTime());
            break;
        }
    }
    if (witnessAccountKeys.empty())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Witness account has no active keys.");

    // FIXME: only unique keys in here using the earliest time for each (so need to introduce a map for this)
    witnessAccountKeys = "gulden://witnesskeys?keys=" + witnessAccountKeys;
    return witnessAccountKeys;
}

static UniValue importwitnesskeys(const JSONRPCRequest& request)
{
    #ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    LOCK2(cs_main, pwallet ? &pwallet->cs_wallet : NULL);
    #else
    LOCK(cs_main);
    #endif
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 3)
        throw std::runtime_error(
            "importwitnesskeys \"account_name\" \"encoded_key_url\" \"create_account\" \n"
            "\nAdd keys imported from an \"encoded_key_url\" which has been obtained by using \"getwitnessaddresskeys\" or \"getwitnessaccountkeys\" \n"
            "\nUses an existing account if \"create_account\" is false, otherwise creates a new one. \n"
            "1. \"account_name\"            (string) name/label of the new account.\n"
            "2. \"encoded_key_url\"         (string) Encoded string containing the extended public key for the account.\n"
            "3. \"create_account\"          (boolean, optional, default=false) Encoded string containing the extended public key for the account.\n"
            "\nResult:\n"
            "\nReturn the UUID of account.\n"
            "\nExamples:\n"
            + HelpExampleCli("importwitnesskeys \"my witness account\" \"gulden://witnesskeys?keys=Vd69eLAZ2r76C47xB3pDLa9Fx4Li8Xt5AHgzjJDuLbkP8eqUjToC#1529049773\"", "")
            + HelpExampleRpc("importwitnesskeys \"my witness account\" \"gulden://witnesskeys?keys=Vd69eLAZ2r76C47xB3pDLa9Fx4Li8Xt5AHgzjJDuLbkP8eqUjToC#1529049773\"", ""));

    if (!pwallet)
        throw std::runtime_error("Cannot use command without an active wallet");

    EnsureWalletIsUnlocked(pwallet);

    bool shouldCreateAccount = false;
    if (request.params.size() > 2)
        shouldCreateAccount = request.params[2].get_bool();

    const auto& keysAndBirthDates = pwallet->ParseWitnessKeyURL(request.params[1].get_str().c_str());
    if (keysAndBirthDates.empty())
        throw std::runtime_error("Invalid encoded key URL");

    CAccount* account = nullptr;
    if (shouldCreateAccount)
    {
        account = AccountFromValue(pwallet, request.params[0], false);
        if (!account)
            throw std::runtime_error("Invalid account name or UUID");
        if (account->m_Type != WitnessOnlyWitnessAccount)
            throw std::runtime_error("Account is not a witness-only account");

        if (!pwallet->ImportKeysIntoWitnessOnlyWitnessAccount(account, keysAndBirthDates))
            throw std::runtime_error("Failed to import keys into account");
    }
    else
    {
        std::string requestedAccountName = request.params[0].get_str();
        account = pwallet->CreateWitnessOnlyWitnessAccount(requestedAccountName, keysAndBirthDates);
        if (!account)
            throw std::runtime_error("Failed to create witness-only witness account");
    }

    //NB! No need to trigger a rescan CreateWitnessOnlyWitnessAccount already did this.

    return getUUIDAsString(account->getUUID());
}


static const CRPCCommand commands[] =
{ //  category                   name                               actor (function)                 okSafeMode
  //  ---------------------      ------------------------           -----------------------          ----------
    { "mining",                  "gethashps",                       &gethashps,                      true,    {} },
    { "mining",                  "sethashlimit",                    &sethashlimit,                   true,    {"limit"} },

    //fixme: (PHASE5) Many of these belong in accounts category as well.
    //We should consider allowing multiple categories for commands, so its easier for people to discover commands under specific topics they are interested in.
    { "witness",                 "createwitnessaccount",            &createwitnessaccount,           true,    {"name"} },
    { "witness",                 "extendwitnessaccount",            &extendwitnessaccount,           true,    {"funding_account", "witness_account", "amount", "time" } },
    { "witness",                 "extendwitnessaddress",            &extendwitnessaddress,           true,    {"funding_account", "witness_address", "amount", "time" } },
    { "witness",                 "fundwitnessaccount",              &fundwitnessaccount,             true,    {"funding_account", "witness_account", "amount", "time", "force_multiple" } },
    { "witness",                 "getwitnessaccountkeys",           &getwitnessaccountkeys,          true,    {"witness_account"} },
    { "witness",                 "getwitnessaddresskeys",           &getwitnessaddresskeys,          true,    {"witness_address"} },
    { "witness",                 "getwitnesscompound",              &getwitnesscompound,             true,    {"witness_account"} },
    { "witness",                 "getwitnessinfo",                  &getwitnessinfo,                 true,    {"block_specifier", "verbose", "mine_only"} },
    { "witness",                 "getwitnessrewardscript",          &getwitnessrewardscript,         true,    {"witness_account"} },
    { "witness",                 "importwitnesskeys",               &importwitnesskeys,              true,    {"account_name", "encoded_key_url", "create_account"} },
    { "witness",                 "mergewitnessaccount",             &mergewitnessaccount,            true,    {"funding_account", "witness_account"} },
    { "witness",                 "rotatewitnessaddress",            &rotatewitnessaddress,           true,    {"funding_account", "witness_address"} },
    { "witness",                 "rotatewitnessaccount",            &rotatewitnessaccount,           true,    {"funding_account", "witness_account"} },
    { "witness",                 "renewwitnessaccount",             &renewwitnessaccount,            true,    {"funding_account", "witness_account"} },
    { "witness",                 "setwitnesscompound",              &setwitnesscompound,             true,    {"witness_account", "amount"} },
    { "witness",                 "setwitnessrewardscript",          &setwitnessrewardscript,         true,    {"witness_account", "pubkey_or_script", "force_pubkey"} },
    { "witness",                 "setwitnessrewardtemplate",        &setwitnessrewardtemplate,       true,    {"witness_account", "reward_template"} },
    { "witness",                 "getwitnessrewardtemplate",        &getwitnessrewardtemplate,       true,    {"witness_account" } },
    { "witness",                 "splitwitnessaccount",             &splitwitnessaccount,            true,    {"funding_account", "witness_account", "amounts"} },
    { "witness",                 "enablewitnessing",                &enablewitnessing,               true,    {} },
    { "witness",                 "disablewitnessing",               &disablewitnessing,              true,    {} },

    { "developer",               "dumpblockgaps",                   &dumpblockgaps,                  true,    {"start_height", "count"} },
    { "developer",               "dumpfiltercheckpoints",           &dumpfiltercheckpoints,          true,    {} },
    { "developer",               "dumptransactionstats",            &dumptransactionstats,           true,    {"start_height", "count"} },
    { "developer",               "dumpdiffarray",                   &dumpdiffarray,                  true,    {"height"} },
    { "developer",               "verifywitnessaddress",            &verifywitnessaddress,           true,    {"witness_address" } },
    { "developer",               "checkpointinvalidate",            &checkpointinvalidate,           true,    {"block_hash" } },

    { "accounts",                "changeaccountname",               &changeaccountname,              true,    {"account", "name"} },
    { "accounts",                "createaccount",                   &createaccount,                  true,    {"name", "type"} },
    { "accounts",                "deleteaccount",                   &deleteaccount,                  true,    {"account", "force"} },
    { "accounts",                "getactiveaccount",                &getactiveaccount,               true,    {} },
    { "accounts",                "getreadonlyaccount",              &getreadonlyaccount,             true,    {"account"} },
    { "accounts",                "importreadonlyaccount",           &importreadonlyaccount,          true,    {"name", "encoded_key"} },
    { "accounts",                "importlinkedaccount",             &importlinkedaccount,            true,    {"name", "encoded_key_uri"} },
    { "accounts",                "listaccounts",                    &listallaccounts,                true,    {"seed", "state"} },
    { "accounts",                "setactiveaccount",                &setactiveaccount,               true,    {"account"} },
    { "accounts",                "getaccountbalances",              &getaccountbalances,             false,   {"min_conf", "include_watchonly"} },

    { "mnemonics",               "createseed",                      &createseed,                     true,    {"type"} },
    { "mnemonics",               "deleteseed",                      &deleteseed,                     true,    {"seed", "should_purge_accounts"} },
    { "mnemonics",               "getactiveseed",                   &getactiveseed,                  true,    {} },
    { "mnemonics",               "getmnemonicfromseed",             &getmnemonicfromseed,            true,    {"seed"} },
    { "mnemonics",               "getreadonlyseed",                 &getreadonlyseed,                true,    {"seed"} },
    { "mnemonics",               "setactiveseed",                   &setactiveseed,                  true,    {"seed"} },
    { "mnemonics",               "importseed",                      &importseed,                     true,    {"mnemonic_or_pubkey", "type", "is_read_only"} },
    { "mnemonics",               "listseeds",                       &listseeds,                      true,    {} },
};

void RegisterGuldenRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}

#endif


