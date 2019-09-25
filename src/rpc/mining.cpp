// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2019 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "base58.h"
#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "consensus/consensus.h"
#include "consensus/params.h"
#include "consensus/validation.h"
#include "validation/validation.h"
#include "validation/validationinterface.h"
#include "validation/versionbitsvalidation.h"
#include "core_io.h"
#include "init.h"
#include "versionbits.h"
#include "generation/miner.h"
#include "net.h"
#include "policy/fees.h"
#include "pow.h"
#include "rpc/blockchain.h"
#include "rpc/server.h"
#include "txmempool.h"
#include "util.h"
#include "utilstrencodings.h"
#include "arith_uint256.h"
#include "warnings.h"
#include "Gulden/util.h"
#include <compat/sys.h>

#include <memory>
#include <stdint.h>

#include <univalue.h>
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif
#include "generation/generation.h"
#include "script/script.h"
#include <Gulden/rpcgulden.h>
#include <validation/witnessvalidation.h>

/**
 * Return average network hashes per second based on the last 'lookup' blocks,
 * or from the last difficulty change if 'lookup' is nonpositive.
 * If 'height' is nonnegative, compute the estimate at the time when a given block was found.
 */
static UniValue GetNetworkHashPS(int lookup, int height) {
    CBlockIndex *pb = chainActive.Tip();

    if (height >= 0 && height < chainActive.Height())
        pb = chainActive[height];

    if (pb == NULL || !pb->nHeight)
        return 0;

    // If lookup is -1, then use blocks since last difficulty change.
    if (lookup <= 0)
        lookup = pb->nHeight % Params().GetConsensus().DifficultyAdjustmentInterval() + 1;

    // If lookup is larger than chain, then set it to chain length.
    if (lookup > pb->nHeight)
        lookup = pb->nHeight;

    CBlockIndex *pb0 = pb;
    int64_t minTime = pb0->GetBlockTime();
    int64_t maxTime = minTime;
    for (int i = 0; i < lookup; i++) {
        pb0 = pb0->pprev;
        int64_t time = pb0->GetBlockTime();
        minTime = std::min(time, minTime);
        maxTime = std::max(time, maxTime);
    }

    // In case there's a situation where minTime == maxTime, we don't want a divide by zero exception.
    if (minTime == maxTime)
        return 0;

    arith_uint256 workDiff = pb->nChainWork - pb0->nChainWork;
    int64_t timeDiff = maxTime - minTime;

    return workDiff.getdouble() / timeDiff;
}

static UniValue getnetworkhashps(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 2)
        throw std::runtime_error(
            "getnetworkhashps ( nblocks height )\n"
            "\nReturns the estimated network hashes per second based on the last n blocks.\n"
            "Pass in [blocks] to override # of blocks, -1 specifies since last difficulty change.\n"
            "Pass in [height] to estimate the network speed at the time when a certain block was found.\n"
            "\nArguments:\n"
            "1. nblocks     (numeric, optional, default=120) The number of blocks, or -1 for blocks since last difficulty change.\n"
            "2. height      (numeric, optional, default=-1) To estimate at the time of the given height.\n"
            "\nResult:\n"
            "x             (numeric) Hashes per second estimated\n"
            "\nExamples:\n"
            + HelpExampleCli("getnetworkhashps", "")
            + HelpExampleRpc("getnetworkhashps", "")
       );

    LOCK(cs_main);
    return GetNetworkHashPS(request.params.size() > 0 ? request.params[0].get_int() : 120, request.params.size() > 1 ? request.params[1].get_int() : -1);
}

static UniValue generateBlocks(std::shared_ptr<CReserveKeyOrScript> coinbaseScript, int nGenerate, uint64_t nMaxTries, bool keepScript)
{
    static const int nInnerLoopCount = 0x10000;
    int nHeightEnd = 0;
    int nHeight = 0;

    {   // Don't keep cs_main locked
        LOCK(cs_main);
        nHeight = chainActive.Height();
        nHeightEnd = nHeight+nGenerate;
    }
    unsigned int nExtraNonce = 0;
    UniValue blockHashes(UniValue::VARR);
    while (nHeight < nHeightEnd)
    {
        std::unique_ptr<CBlockTemplate> pblocktemplate(BlockAssembler(Params()).CreateNewBlock(chainActive.Tip(), coinbaseScript));
        if (!pblocktemplate.get())
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Couldn't create new block");
        CBlock *pblock = &pblocktemplate->block;
        {
            LOCK(cs_main);
            IncrementExtraNonce(pblock, chainActive.Tip(), nExtraNonce);
        }
        while (nMaxTries > 0 && pblock->nNonce < nInnerLoopCount && !CheckProofOfWork(pblock, Params().GetConsensus())) {
            ++pblock->nNonce;
            --nMaxTries;
        }
        if (nMaxTries == 0) {
            break;
        }
        if (pblock->nNonce == nInnerLoopCount) {
            continue;
        }
        std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
        if (!ProcessNewBlock(Params(), shared_pblock, true, NULL, false, true))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "ProcessNewBlock, block not accepted");
        ++nHeight;
        blockHashes.push_back(pblock->GetHashLegacy().GetHex());

        //mark script as important because it was used at least for one coinbase output if the script came from the wallet
        if (keepScript)
        {
            coinbaseScript->KeepScript();
        }
    }
    return blockHashes;
}

void InitRPCMining()
{
}

void ShutdownRPCMining()
{
}

static UniValue getgenerate(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getgenerate\n"
            "\nReturn if the server is set to generate coins or not. The default is false.\n"
            "It is set with the command line argument -gen (or " + std::string(GULDEN_CONF_FILENAME) + " setting gen)\n"
            "It can also be set with the setgenerate call.\n"
            "\nResult\n"
            "true|false      (boolean) If the server is set to generate coins or not\n"
            "\nExamples:\n"
            + HelpExampleCli("getgenerate", "")
            + HelpExampleRpc("getgenerate", "")
        );

    LOCK(cs_main);
    return GetBoolArg("-gen", DEFAULT_GENERATE);
}

static UniValue generate(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "generate num_blocks ( max_tries )\n"
            "\ngenerate up to n blocks immediately (before the RPC call returns)\n"
            "\nArguments:\n"
            "1. nblocks      (numeric, required) How many blocks are generated immediately.\n"
            "2. maxtries     (numeric, optional) How many iterations to try (default = 1000000).\n"
            "\nResult:\n"
            "[ blockhashes ]     (array) hashes of blocks generated\n"
            "\nExamples:\n"
            "\nGenerate 11 blocks\n"
            + HelpExampleCli("generate", "11")
        );

    int nGenerate = request.params[0].get_int();
    uint64_t nMaxTries = 1000000;
    if (request.params.size() > 1) {
        nMaxTries = request.params[1].get_int();
    }

    std::shared_ptr<CReserveKeyOrScript> coinbaseScript;
    GetMainSignals().ScriptForMining(coinbaseScript, NULL);

    // If the keypool is exhausted, no script is returned at all.  Catch this.
    if (!coinbaseScript)
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");

    //throw an error if no script was provided
    if (coinbaseScript->reserveScript.empty())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "No coinbase script available; a wallet is required");

    return generateBlocks(coinbaseScript, nGenerate, nMaxTries, true);
}

static UniValue setgenerate(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 4)
        throw std::runtime_error(
            "setgenerate generate ( gen_proc_limit )\n"
            "\nSet 'generate' true or false to turn generation on or off.\n"
            "Generation is limited to 'gen_proc_limit' processors, -1 is unlimited.\n"
            "See the getgenerate call for the current setting.\n"
            "\nArguments:\n"
            "1. generate         (boolean, required) Set to true to turn on generation, off to turn off.\n"
            "2. gen_proc_limit   (numeric, optional) Set the processor limit for when generation is on. Can be -1 for unlimited.\n"
            "3. gen_memory_limit (numeric, optional) How much system memory to use. -1 to use system default\n"
            "4. account          (string, optional) The UUID or unique label of the account.\n"
            "\nExamples:\n"
            "\nSet the generation on with a limit of one processor\n"
            + HelpExampleCli("setgenerate", "true 1") +
            "\nCheck the setting\n"
            + HelpExampleCli("getgenerate", "") +
            "\nTurn off generation\n"
            + HelpExampleCli("setgenerate", "false") +
            "\nUsing json rpc\n"
            + HelpExampleRpc("setgenerate", "true, 1")
        );

    if (Params().MineBlocksOnDemand())
        throw JSONRPCError(RPC_METHOD_NOT_FOUND, "Use the generate method instead of setgenerate on this network");

    #ifdef ENABLE_WALLET
    CWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!pwallet)
    {
        throw std::runtime_error("Cannot use command without an active wallet");
    }

    bool fGenerate = true;
    if (request.params.size() > 0)
    {
        fGenerate = request.params[0].get_bool();
    }

    CAccount* forAccount = nullptr;
    if (request.params.size() > 3)
    {
        forAccount = AccountFromValue(pactiveWallet, request.params[3], false);
    }
    else
    {
        if (!pactiveWallet->activeAccount)
        {
            throw std::runtime_error("No active account selected, first select an active account.");
        }
        forAccount = pactiveWallet->activeAccount;
    }

    if (fGenerate && forAccount->IsPoW2Witness())
    {
        throw std::runtime_error("Witness account selected, first select a regular account as the active account or specifiy a regular account.");
    }

    int nGenProcLimit = GetArg("-genproclimit", DEFAULT_GENERATE_THREADS);
    if (request.params.size() > 1)
    {
        nGenProcLimit = request.params[1].get_int();
        if (nGenProcLimit == 0)
        {
            fGenerate = false;
        }
    }

    // Try to avoid swap by never using more than sysmem-1gb or on machines with only 1gb of memory sysmem-512mb.
    uint64_t systemMemory = systemPhysicalMemoryInBytes();
    if (systemMemory > 1 * 1024 * 1024 * 1024)
    {
        systemMemory -= 1 * 1024 * 1024 * 1024;
    }
    else
    {
        systemMemory -= 512 * 1024 * 1024;
    }

    // Allow user to override default memory selection.
    int64_t nGenMemoryLimit = std::min(systemMemory, defaultSigmaSettings.arenaSizeKb*1024);
    if (request.params.size() > 2)
    {
        nGenMemoryLimit = request.params[2].get_int();
        if (nGenMemoryLimit == 0)
        {
            fGenerate = false;
        }
        if (nGenMemoryLimit < 0)
        {
            nGenMemoryLimit = systemMemory;
        }
    }
    
    SoftSetBoolArg("-gen", fGenerate);
    SoftSetArg("-genproclimit", itostr(nGenProcLimit));
    PoWMineGulden(fGenerate, nGenProcLimit, nGenMemoryLimit/1024, Params(), forAccount);

    if (!fGenerate)
    {
        return "Block generation disabled.";
    }
    else
    {
        return strprintf("Block generation enabled into account [%s], thread limit: [%d threads], memory: [%d Mb].", pwallet->mapAccountLabels[forAccount->getUUID()] ,nGenProcLimit, nGenMemoryLimit/1024/1024);
    }
    #else
    throw std::runtime_error("Cannot use command without an active wallet");
    return nullptr;
    #endif
}

static UniValue generatetoaddress(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 3)
        throw std::runtime_error(
            "generatetoaddress num_blocks address (max_tries)\n"
            "\nGenerate blocks immediately to a specified address (before the RPC call returns)\n"
            "\nArguments:\n"
            "1. nblocks      (numeric, required) How many blocks are generated immediately.\n"
            "2. address      (string, required) The address to send the newly generated Gulden to.\n"
            "3. maxtries     (numeric, optional) How many iterations to try (default = 1000000).\n"
            "\nResult:\n"
            "[ blockhashes ]     (array) hashes of blocks generated\n"
            "\nExamples:\n"
            "\nGenerate 11 blocks to myaddress\n"
            + HelpExampleCli("generatetoaddress", "11 \"myaddress\"")
        );

    int nGenerate = request.params[0].get_int();
    uint64_t nMaxTries = 1000000;
    if (request.params.size() > 2) {
        nMaxTries = request.params[2].get_int();
    }

    CGuldenAddress address(request.params[1].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Error: Invalid address");

    std::shared_ptr<CReserveKeyOrScript> coinbaseScript = std::make_shared<CReserveKeyOrScript>();
    coinbaseScript->reserveScript = GetScriptForDestination(address.Get());

    return generateBlocks(coinbaseScript, nGenerate, nMaxTries, false);
}

static UniValue getmininginfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getmininginfo\n"
            "\nReturns a json object containing information about block generation."
            "\nResult:\n"
            "{\n"
            "  \"blocks\": nnn,             (numeric) The current block\n"
            "  \"currentblocksize\": nnn,   (numeric) The last block size\n"
            "  \"currentblockweight\": nnn, (numeric) The last block weight\n"
            "  \"currentblocktx\": nnn,     (numeric) The last block transaction\n"
            "  \"difficulty\": xxx.xxxxx    (numeric) The current difficulty\n"
            "  \"errors\": \"...\"            (string) Current errors\n"
            "  \"networkhashps\": nnn,      (numeric) The network hashes per second\n"
            "  \"generate\": true|false     (boolean) If the generation is on or off (see getgenerate or setgenerate calls)\n"
            "  \"genproclimit\": n          (numeric) The processor limit for generation. -1 if no generation. (see getgenerate or setgenerate calls)\n"
            "  \"pooledtx\": n              (numeric) The size of the mempool\n"
            "  \"chain\": \"xxxx\",           (string) current network name as defined in BIP70 (main, test, regtest)\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getmininginfo", "")
            + HelpExampleRpc("getmininginfo", "")
        );


    LOCK(cs_main);

    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("blocks",           (int)chainActive.Height()));
    obj.push_back(Pair("currentblocksize", (uint64_t)nLastBlockSize));
    obj.push_back(Pair("currentblockweight", (uint64_t)nLastBlockWeight));
    obj.push_back(Pair("currentblocktx",   (uint64_t)nLastBlockTx));
    obj.push_back(Pair("difficulty",       (double)GetDifficulty()));
    obj.push_back(Pair("errors",           GetWarnings("statusbar")));
    obj.push_back(Pair("genproclimit",     (int)GetArg("-genproclimit", DEFAULT_GENERATE_THREADS)));
    obj.push_back(Pair("networkhashps",    getnetworkhashps(request)));
    obj.push_back(Pair("pooledtx",         (uint64_t)mempool.size()));
    obj.push_back(Pair("chain",            Params().NetworkIDString()));
    obj.push_back(Pair("generate",         getgenerate(request)));
    return obj;
}


// NOTE: Unlike wallet RPC (which use NLG values), mining RPCs follow GBT (BIP 22) in using satoshi amounts
static UniValue prioritisetransaction(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 3)
        throw std::runtime_error(
            "prioritisetransaction <txid> <dummy_value> <fee_delta>\n"
            "Accepts the transaction into generated blocks at a higher (or lower) priority\n"
            "\nArguments:\n"
            "1. \"txid\"         (string, required) The transaction id.\n"
            "2. dummy_value    (numeric, optional) API-Compatibility for previous API. Must be zero or null.\n"
            "                  DEPRECATED. For forward compatibility use named arguments and omit this parameter.\n"
            "3. fee_delta      (numeric, required) The fee value (in satoshis) to add (or subtract, if negative).\n"
            "                  The fee is not actually paid, only the algorithm for selecting transactions into a block\n"
            "                  considers the transaction as it would have paid a higher (or lower) fee.\n"
            "\nResult:\n"
            "true              (boolean) Returns true\n"
            "\nExamples:\n"
            + HelpExampleCli("prioritisetransaction", "\"txid\" 0.0 10000")
            + HelpExampleRpc("prioritisetransaction", "\"txid\", 0.0, 10000")
        );

    LOCK(cs_main);

    uint256 hash = ParseHashStr(request.params[0].get_str(), "txid");
    CAmount nAmount = request.params[2].get_int64();

    if (!(request.params[1].isNull() || request.params[1].get_real() == 0)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Priority is no longer supported, dummy argument to prioritisetransaction must be 0.");
    }

    mempool.PrioritiseTransaction(hash, nAmount);
    return true;
}


// NOTE: Assumes a conclusive result; if result is inconclusive, it must be handled by caller
static UniValue BIP22ValidationResult(const CValidationState& state)
{
    if (state.IsValid())
        return NullUniValue;

    std::string strRejectReason = state.GetRejectReason();
    if (state.IsError())
        throw JSONRPCError(RPC_VERIFY_ERROR, strRejectReason);
    if (state.IsInvalid())
    {
        if (strRejectReason.empty())
            return "rejected";
        return strRejectReason;
    }
    // Should be impossible
    return "valid?";
}

class submitblock_StateCatcher : public CValidationInterface
{
public:
    uint256 hash;
    bool found;
    CValidationState state;

    submitblock_StateCatcher(const uint256 &hashIn) : hash(hashIn), found(false), state() {}

protected:
    void BlockChecked(const CBlock& block, const CValidationState& stateIn) override {
        if (block.GetHashPoW2() != hash)
            return;
        found = true;
        state = stateIn;
    }
};

static UniValue submitblock(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2) {
        throw std::runtime_error(
            "submitblock \"hexdata\" ( \"jsonparametersobject\" )\n"
            "\nAttempts to submit new block to network.\n"
            "The 'jsonparametersobject' parameter is currently ignored.\n"
            "See https://en.bitcoin.it/wiki/BIP_0022 for full specification.\n"

            "\nArguments\n"
            "1. \"hexdata\"        (string, required) the hex-encoded block data to submit\n"
            "2. \"parameters\"     (string, optional) object of optional parameters\n"
            "    {\n"
            "      \"workid\" : \"id\"    (string, optional) if the server provided a workid, it MUST be included with submissions\n"
            "    }\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("submitblock", "\"mydata\"")
            + HelpExampleRpc("submitblock", "\"mydata\"")
        );
    }

    std::shared_ptr<CBlock> blockptr = std::make_shared<CBlock>();
    CBlock& block = *blockptr;
    if (!DecodeHexBlk(block, request.params[0].get_str())) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Block decode failed");
    }

    if (block.vtx.empty() || !block.vtx[0]->IsCoinBase()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Block does not start with a coinbase");
    }

    uint256 hash = block.GetHashPoW2();
    bool fBlockPresent = false;
    {
        LOCK(cs_main);
        BlockMap::iterator mi = mapBlockIndex.find(hash);
        if (mi != mapBlockIndex.end()) {
            CBlockIndex *pindex = mi->second;
            if (pindex->IsValid(BLOCK_VALID_SCRIPTS)) {
                return "duplicate";
            }
            if (pindex->nStatus & BLOCK_FAILED_MASK) {
                return "duplicate-invalid";
            }
            // Otherwise, we might only have the header - process the block before returning
            fBlockPresent = true;
        }
    }

    submitblock_StateCatcher sc(block.GetHashPoW2());
    RegisterValidationInterface(&sc);
    bool fAccepted = ProcessNewBlock(Params(), blockptr, true, NULL, false, true);
    UnregisterValidationInterface(&sc);


    if (fBlockPresent) {
        if (fAccepted && !sc.found) {
            return "duplicate-inconclusive";
        }
        return "duplicate";
    }
    if (!fAccepted)
        return "invalid";
    if (!sc.found) {
        return "inconclusive";
    }
    return BIP22ValidationResult(sc.state);
}

static UniValue estimatefee(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "estimatefee num_blocks\n"
            "\nDEPRECATED. Please use estimatesmartfee for more intelligent estimates."
            "\nEstimates the approximate fee per kilobyte needed for a transaction to begin\n"
            "confirmation within num_blocks blocks. Uses virtual transaction size of transaction\n"
            "as defined in BIP 141 (witness data is discounted).\n"
            "\nArguments:\n"
            "1. num_blocks  (numeric, required)\n"
            "\nResult:\n"
            "n              (numeric) estimated fee-per-kilobyte\n"
            "\n"
            "A negative value is returned if not enough transactions and blocks\n"
            "have been observed to make an estimate.\n"
            "-1 is always returned for num_blocks == 1 as it is impossible to calculate\n"
            "a fee that is high enough to get reliably included in the next block.\n"
            "\nExample:\n"
            + HelpExampleCli("estimatefee", "6")
            );

    RPCTypeCheck(request.params, {UniValue::VNUM});

    int nBlocks = request.params[0].get_int();
    if (nBlocks < 1)
        nBlocks = 1;

    CFeeRate feeRate = ::feeEstimator.estimateFee(nBlocks);
    if (feeRate == CFeeRate(0))
        return -1.0;

    return ValueFromAmount(feeRate.GetFeePerK());
}

static UniValue estimatesmartfee(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "estimatesmartfee num_blocks (conservative)\n"
            "\nEstimates the approximate fee per kilobyte needed for a transaction to begin\n"
            "confirmation within num_blocks blocks if possible and return the number of blocks\n"
            "for which the estimate is valid. Uses virtual transaction size as defined\n"
            "in BIP 141 (witness data is discounted).\n"
            "\nArguments:\n"
            "1. num_blocks    (numeric)\n"
            "2. conservative  (bool, optional, default=true) Whether to return a more conservative estimate which\n"
            "                 also satisfies a longer history. A conservative estimate potentially returns a higher\n"
            "                 feerate and is more likely to be sufficient for the desired target, but is not as\n"
            "                 responsive to short term drops in the prevailing fee market\n"
            "\nResult:\n"
            "{\n"
            "  \"feerate\" : x.x,     (numeric) estimate fee-per-kilobyte (in NLG)\n"
            "  \"blocks\" : n         (numeric) block number where estimate was found\n"
            "}\n"
            "\n"
            "A negative value is returned if not enough transactions and blocks\n"
            "have been observed to make an estimate for any number of blocks.\n"
            "However it will not return a value below the mempool reject fee.\n"
            "\nExample:\n"
            + HelpExampleCli("estimatesmartfee", "6")
            );

    RPCTypeCheck(request.params, {UniValue::VNUM});

    int nBlocks = request.params[0].get_int();
    bool conservative = true;
    if (request.params.size() > 1 && !request.params[1].isNull()) {
        RPCTypeCheckArgument(request.params[1], UniValue::VBOOL);
        conservative = request.params[1].get_bool();
    }

    UniValue result(UniValue::VOBJ);
    int answerFound;
    CFeeRate feeRate = ::feeEstimator.estimateSmartFee(nBlocks, &answerFound, ::mempool, conservative);
    result.push_back(Pair("feerate", feeRate == CFeeRate(0) ? -1.0 : ValueFromAmount(feeRate.GetFeePerK())));
    result.push_back(Pair("blocks", answerFound));
    return result;
}

static UniValue estimaterawfee(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1|| request.params.size() > 3)
        throw std::runtime_error(
            "estimaterawfee num_blocks (threshold horizon)\n"
            "\nWARNING: This interface is unstable and may disappear or change!\n"
            "\nWARNING: This is an advanced API call that is tightly coupled to the specific\n"
            "         implementation of fee estimation. The parameters it can be called with\n"
            "         and the results it returns will change if the internal implementation changes.\n"
            "\nEstimates the approximate fee per kilobyte needed for a transaction to begin\n"
            "confirmation within num_blocks blocks if possible. Uses virtual transaction size as defined\n"
            "in BIP 141 (witness data is discounted).\n"
            "\nArguments:\n"
            "1. num_blocks  (numeric)\n"
            "2. threshold   (numeric, optional) The proportion of transactions in a given feerate range that must have been\n"
            "               confirmed within num_blocks in order to consider those feerates as high enough and proceed to check\n"
            "               lower buckets.  Default: 0.95\n"
            "3. horizon     (numeric, optional) How long a history of estimates to consider. 0=short, 1=medium, 2=long.\n"
            "               Default: 1\n"
            "\nResult:\n"
            "{\n"
            "  \"feerate\" : x.x,        (numeric) estimate fee-per-kilobyte (in NLG)\n"
            "  \"decay\" : x.x,          (numeric) exponential decay (per block) for historical moving average of confirmation data\n"
            "  \"scale\" : x,            (numeric) The resolution of confirmation targets at this time horizon\n"
            "  \"pass\" : {              (json object) information about the lowest range of feerates to succeed in meeting the threshold\n"
            "      \"startrange\" : x.x,     (numeric) start of feerate range\n"
            "      \"endrange\" : x.x,       (numeric) end of feerate range\n"
            "      \"withintarget\" : x.x,   (numeric) number of txs over history horizon in the feerate range that were confirmed within target\n"
            "      \"totalconfirmed\" : x.x, (numeric) number of txs over history horizon in the feerate range that were confirmed at any point\n"
            "      \"inmempool\" : x.x,      (numeric) current number of txs in mempool in the feerate range unconfirmed for at least target blocks\n"
            "      \"leftmempool\" : x.x,    (numeric) number of txs over history horizon in the feerate range that left mempool unconfirmed after target\n"
            "  }\n"
            "  \"fail\" : { ... }        (json object) information about the highest range of feerates to fail to meet the threshold\n"
            "}\n"
            "\n"
            "A negative feerate is returned if no answer can be given.\n"
            "\nExample:\n"
            + HelpExampleCli("estimaterawfee", "6 0.9 1")
            );

    RPCTypeCheck(request.params, {UniValue::VNUM, UniValue::VNUM, UniValue::VNUM}, true);
    RPCTypeCheckArgument(request.params[0], UniValue::VNUM);
    int nBlocks = request.params[0].get_int();
    double threshold = 0.95;
    if (!request.params[1].isNull())
        threshold = request.params[1].get_real();
    FeeEstimateHorizon horizon = FeeEstimateHorizon::MED_HALFLIFE;
    if (!request.params[2].isNull()) {
        int horizonInt = request.params[2].get_int();
        if (horizonInt < 0 || horizonInt > 2) {
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid horizon for fee estimates");
        } else {
            horizon = (FeeEstimateHorizon)horizonInt;
        }
    }
    UniValue result(UniValue::VOBJ);
    CFeeRate feeRate;
    EstimationResult buckets;
    feeRate = ::feeEstimator.estimateRawFee(nBlocks, threshold, horizon, &buckets);

    result.push_back(Pair("feerate", feeRate == CFeeRate(0) ? -1.0 : ValueFromAmount(feeRate.GetFeePerK())));
    result.push_back(Pair("decay", buckets.decay));
    result.push_back(Pair("scale", (int)buckets.scale));
    UniValue passbucket(UniValue::VOBJ);
    passbucket.push_back(Pair("startrange", round(buckets.pass.start)));
    passbucket.push_back(Pair("endrange", round(buckets.pass.end)));
    passbucket.push_back(Pair("withintarget", round(buckets.pass.withinTarget * 100.0) / 100.0));
    passbucket.push_back(Pair("totalconfirmed", round(buckets.pass.totalConfirmed * 100.0) / 100.0));
    passbucket.push_back(Pair("inmempool", round(buckets.pass.inMempool * 100.0) / 100.0));
    passbucket.push_back(Pair("leftmempool", round(buckets.pass.leftMempool * 100.0) / 100.0));
    result.push_back(Pair("pass", passbucket));
    UniValue failbucket(UniValue::VOBJ);
    failbucket.push_back(Pair("startrange", round(buckets.fail.start)));
    failbucket.push_back(Pair("endrange", round(buckets.fail.end)));
    failbucket.push_back(Pair("withintarget", round(buckets.fail.withinTarget * 100.0) / 100.0));
    failbucket.push_back(Pair("totalconfirmed", round(buckets.fail.totalConfirmed * 100.0) / 100.0));
    failbucket.push_back(Pair("inmempool", round(buckets.fail.inMempool * 100.0) / 100.0));
    failbucket.push_back(Pair("leftmempool", round(buckets.fail.leftMempool * 100.0) / 100.0));
    result.push_back(Pair("fail", failbucket));
    return result;
}

static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         okSafeMode
  //  --------------------- ------------------------  -----------------------  ----------
    { "block_generation",   "getnetworkhashps",       &getnetworkhashps,       true,  {"num_blocks","height"} },
    { "block_generation",   "getmininginfo",          &getmininginfo,          true,  {} },
    { "block_generation",   "prioritisetransaction",  &prioritisetransaction,  true,  {"txid","dummy_value","fee_delta"} },
    { "block_generation",   "submitblock",            &submitblock,            true,  {"hexdata","parameters"} },

    { "generating",         "generate",               &generate,               true,  {"num_blocks","max_tries"} },
    { "generating",         "generatetoaddress",      &generatetoaddress,      true,  {"num_blocks","address","max_tries"} },
    { "generating",         "getgenerate",            &getgenerate,            true,  {}  },
    { "generating",         "setgenerate",            &setgenerate,            true,  {"generate", "gen_proc_limit", "gen_memory_limit"}  },

    { "util",               "estimatefee",            &estimatefee,            true,  {"num_blocks"} },
    { "util",               "estimatesmartfee",       &estimatesmartfee,       true,  {"num_blocks", "conservative"} },

    { "hidden",             "estimaterawfee",         &estimaterawfee,         true,  {"num_blocks", "threshold", "horizon"} },
};

void RegisterMiningRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
