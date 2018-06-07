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

#include "base58.h"
#include "chain.h"
#include "rpc/server.h"
#include "init.h"
#include "validation.h"
#include "script/script.h"
#include "script/standard.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "wallet.h"
#include "walletdb.h"
#include "merkleblock.h"
#include "core_io.h"
#include "rpcwallet.h"

#include "rpcwallet.h"

#include <fstream>
#include <stdint.h>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <univalue.h>



std::string static EncodeDumpTime(int64_t nTime) {
    return DateTimeStrFormat("%Y-%m-%dT%H:%M:%SZ", nTime);
}

int64_t static DecodeDumpTime(const std::string &str) {
    static const boost::posix_time::ptime epoch = boost::posix_time::from_time_t(0);
    static const std::locale loc(std::locale::classic(),
        new boost::posix_time::time_input_facet("%Y-%m-%dT%H:%M:%SZ"));
    std::istringstream iss(str);
    iss.imbue(loc);
    boost::posix_time::ptime ptime(boost::date_time::not_a_date_time);
    iss >> ptime;
    if (ptime.is_not_a_date_time())
        return 0;
    return (ptime - epoch).total_seconds();
}

std::string static EncodeDumpString(const std::string &str) {
    std::stringstream ret;
    for(unsigned char c : str) {
        if (c <= 32 || c >= 128 || c == '%') {
            ret << '%' << HexStr(&c, &c + 1);
        } else {
            ret << c;
        }
    }
    return ret.str();
}

std::string DecodeDumpString(const std::string &str) {
    std::stringstream ret;
    for (unsigned int pos = 0; pos < str.length(); pos++) {
        unsigned char c = str[pos];
        if (c == '%' && pos+2 < str.length()) {
            c = (((str[pos+1]>>6)*9+((str[pos+1]-'0')&15)) << 4) | 
                ((str[pos+2]>>6)*9+((str[pos+2]-'0')&15));
            pos += 2;
        }
        ret << c;
    }
    return ret.str();
}

UniValue importprivkey(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 4)
        throw std::runtime_error(
            "importprivkey \"guldenprivkey\" \"account\" ( \"label\" ) ( rescan )\n"
            "\nAdds a private key (as returned by dumpprivkey) to your wallet.\n"
            "\nArguments:\n"
            "1. \"guldenprivkey\"   (string, required) The private key (see dumpprivkey)\n"
            "2. \"account\"          (string, required) The account in which to import the key. \"\" for the currently active account. NB! Must be a legacy account, cannot import keys into an HD account.\n"
            "3. \"label\"            (string, optional, default=\"\") An optional label\n"
            "4. rescan               (boolean, optional, default=true) Rescan the wallet for transactions\n"
            "\nNote: This call can take minutes to complete if rescan is true.\n"
            "\nExamples:\n"
            "\nDump a private key\n"
            + HelpExampleCli("dumpprivkey", "\"myaddress\"") +
            "\nImport the private key with rescan\n"
            + HelpExampleCli("importprivkey", "\"mykey\"") +
            "\nImport using a label and without rescan\n"
            + HelpExampleCli("importprivkey", "\"mykey\" \"testing\" false") +
            "\nImport using default blank label and without rescan\n"
            + HelpExampleCli("importprivkey", "\"mykey\" \"\" false") +
            "\nAs a JSON-RPC call\n"
            + HelpExampleRpc("importprivkey", "\"mykey\", \"testing\", false")
        );


    LOCK2(cs_main, pwallet->cs_wallet);

    EnsureWalletIsUnlocked(pwallet);

    std::string strSecret = request.params[0].get_str();
    std::string strLabel = "";
    if (request.params.size() > 2)
        strLabel = request.params[2].get_str();

    CAccount* forAccount = AccountFromValue(pwallet, request.params[1], true);

    if (forAccount->IsHD())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot import a privkey into an HD account.");

    // Whether to perform rescan after import
    bool fRescan = true;
    if (request.params.size() > 3)
        fRescan = request.params[3].get_bool();

    if (fRescan && fPruneMode)
        throw JSONRPCError(RPC_WALLET_ERROR, "Rescan is disabled in pruned mode");

    CGuldenSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key encoding");

    CKey key = vchSecret.GetKey();
    if (!key.IsValid()) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");

    CPubKey pubkey = key.GetPubKey();
    assert(key.VerifyPubKey(pubkey));
    CKeyID vchAddress = pubkey.GetID();
    {
        pwallet->MarkDirty();
        pwallet->SetAddressBook(CGuldenAddress(vchAddress).ToString(), strLabel, "receive");

        // Don't throw error in case a key is already there
        if (pwallet->HaveKey(vchAddress)) {
            return NullUniValue;
        }

        pwallet->mapKeyMetadata[vchAddress].nCreateTime = 1;

        if (!pwallet->AddKeyPubKey(key, pubkey, *forAccount, KEYCHAIN_EXTERNAL)) {
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");
        }

        // whenever a key is imported, we need to scan the whole chain
        pwallet->UpdateTimeFirstKey(1);

        if (fRescan) {
            pwallet->ScanForWalletTransactions(chainActive.Genesis(), true);
        }
    }

    return NullUniValue;
}

UniValue abortrescan(const JSONRPCRequest& request)
{
    CWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 0)
        throw std::runtime_error(
            "abortrescan\n"
            "\nStops current wallet rescan triggered e.g. by an importprivkey call.\n"
            "\nExamples:\n"
            "\nImport a private key\n"
            + HelpExampleCli("importprivkey", "\"mykey\"") +
            "\nAbort the running wallet rescan\n"
            + HelpExampleCli("abortrescan", "") +
            "\nAs a JSON-RPC call\n"
            + HelpExampleRpc("abortrescan", "")
        );

    if (!pwallet->IsScanning() || pwallet->IsAbortingRescan()) return false;
    pwallet->AbortRescan();
    return true;
}

void ImportAddress(CWallet*, const CGuldenAddress& address, const std::string& strLabel);
void ImportScript(CWallet* const pwallet, const CScript& script, const std::string& strLabel, bool isRedeemScript)
{
    if (!isRedeemScript && ::IsMine(*pwallet, script) == ISMINE_SPENDABLE) {
        throw JSONRPCError(RPC_WALLET_ERROR, "The wallet already contains the private key for this address or script");
    }

    pwallet->MarkDirty();

    //fixme: (Post-2.1) (MERGE) (Watchonly)
    if (!pwallet->HaveWatchOnly(script) && !pwallet->AddWatchOnly(script, -1)) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
    }

    if (isRedeemScript) {
        if (!pwallet->HaveCScript(script) && !pwallet->AddCScript(script)) {
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding p2sh redeemScript to wallet");
        }
        ImportAddress(pwallet, CGuldenAddress(CScriptID(script)), strLabel);
    } else {
        CTxDestination destination;
        if (ExtractDestination(script, destination)) {
            pwallet->SetAddressBook(CGuldenAddress(destination).ToString(), strLabel, "receive");
        }
    }
}

void ImportAddress(CWallet* const pwallet, const CGuldenAddress& address, const std::string& strLabel)
{
    CScript script = GetScriptForDestination(address.Get());
    ImportScript(pwallet, script, strLabel, false);
    // add to address book or update label
    if (address.IsValid())
        pwallet->SetAddressBook(address.ToString(), strLabel, "receive");
}

UniValue importaddress(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 4)
        throw std::runtime_error(
            "importaddress \"address\" ( \"label\" rescan p2sh )\n"
            "\nAdds a script (in hex) or address that can be watched as if it were in your wallet but cannot be used to spend.\n"
            "\nArguments:\n"
            "1. \"script\"           (string, required) The hex-encoded script (or address)\n"
            "2. \"label\"            (string, optional, default=\"\") An optional label\n"
            "3. rescan               (boolean, optional, default=true) Rescan the wallet for transactions\n"
            "4. p2sh                 (boolean, optional, default=false) Add the P2SH version of the script as well\n"
            "\nNote: This call can take minutes to complete if rescan is true.\n"
            "If you have the full public key, you should call importpubkey instead of this.\n"
            "\nNote: If you import a non-standard raw script in hex form, outputs sending to it will be treated\n"
            "as change, and not show up in many RPCs.\n"
            "\nExamples:\n"
            "\nImport a script with rescan\n"
            + HelpExampleCli("importaddress", "\"myscript\"") +
            "\nImport using a label without rescan\n"
            + HelpExampleCli("importaddress", "\"myscript\" \"testing\" false") +
            "\nAs a JSON-RPC call\n"
            + HelpExampleRpc("importaddress", "\"myscript\", \"testing\", false")
        );


    std::string strLabel = "";
    //fixme: (Post-2.1)
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Sorry for the inconvenience, watch only addresses are temporarily disabled but will come back in a future release.");

    if (request.params.size() > 1)
        strLabel = request.params[1].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (request.params.size() > 2)
        fRescan = request.params[2].get_bool();

    if (fRescan && fPruneMode)
        throw JSONRPCError(RPC_WALLET_ERROR, "Rescan is disabled in pruned mode");

    // Whether to import a p2sh version, too
    bool fP2SH = false;
    if (request.params.size() > 3)
        fP2SH = request.params[3].get_bool();

    LOCK2(cs_main, pwallet->cs_wallet);

    CGuldenAddress address(request.params[0].get_str());
    if (address.IsValid()) {
        if (fP2SH)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Cannot use the p2sh flag with an address - use a script instead");
        ImportAddress(pwallet, address, strLabel);
    } else if (IsHex(request.params[0].get_str())) {
        std::vector<unsigned char> data(ParseHex(request.params[0].get_str()));
        ImportScript(pwallet, CScript(data.begin(), data.end()), strLabel, fP2SH);
    } else {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Gulden address or script");
    }

    if (fRescan)
    {
        pwallet->ScanForWalletTransactions(chainActive.Genesis(), true);
        pwallet->ReacceptWalletTransactions();
    }

    return NullUniValue;
}

//fixme: (2.1)
#include "Gulden/util.h"
#include "validation.h"

UniValue importprunedfunds(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "importprunedfunds\n"
            "\nImports funds without rescan. Corresponding address or script must previously be included in wallet. Aimed towards pruned wallets. The end-user is responsible to import additional transactions that subsequently spend the imported outputs or rescan after the point in the blockchain the transaction is included.\n"
            "\nArguments:\n"
            "1. \"rawtransaction\" (string, required) A raw transaction in hex funding an already-existing address in wallet\n"
            "2. \"txoutproof\"     (string, required) The hex output from gettxoutproof that contains the transaction\n"
        );

    CMutableTransaction tx(CURRENT_TX_VERSION_POW2);
    if (!DecodeHexTx(tx, request.params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    uint256 hashTx = tx.GetHash();
    CWalletTx wtx(pwallet, MakeTransactionRef(std::move(tx)));

    CDataStream ssMB(ParseHexV(request.params[1], "proof"), SER_NETWORK, PROTOCOL_VERSION);
    CMerkleBlock merkleBlock;
    ssMB >> merkleBlock;

    //Search partial merkle tree in proof for our transaction and index in valid block
    std::vector<uint256> vMatch;
    std::vector<unsigned int> vIndex;
    unsigned int txnIndex = 0;
    if (merkleBlock.txn.ExtractMatches(vMatch, vIndex) == merkleBlock.header.hashMerkleRoot) {

        LOCK(cs_main);

        if (!mapBlockIndex.count(merkleBlock.header.GetHashPoW2()) || !chainActive.Contains(mapBlockIndex[merkleBlock.header.GetHashPoW2()]))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found in chain");

        std::vector<uint256>::const_iterator it;
        if ((it = std::find(vMatch.begin(), vMatch.end(), hashTx))==vMatch.end()) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Transaction given doesn't exist in proof");
        }

        txnIndex = vIndex[it - vMatch.begin()];
    }
    else {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Something wrong with merkleblock");
    }

    wtx.nIndex = txnIndex;
    wtx.hashBlock = merkleBlock.header.GetHashPoW2();

    LOCK2(cs_main, pwallet->cs_wallet);

    if (pwallet->IsMine(wtx)) {
        pwallet->AddToWallet(wtx, false);
        return NullUniValue;
    }

    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No addresses in wallet correspond to included transaction");
}

UniValue removeprunedfunds(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "removeprunedfunds \"txid\"\n"
            "\nDeletes the specified transaction from the wallet. Meant for use with pruned wallets and as a companion to importprunedfunds. This will affect wallet balances.\n"
            "\nArguments:\n"
            "1. \"txid\"           (string, required) The hex-encoded id of the transaction you are deleting\n"
            "\nExamples:\n"
            + HelpExampleCli("removeprunedfunds", "\"a8d0c0184dde994a09ec054286f1ce581bebf46446a512166eae7628734ea0a5\"") +
            "\nAs a JSON-RPC call\n"
            + HelpExampleRpc("removprunedfunds", "\"a8d0c0184dde994a09ec054286f1ce581bebf46446a512166eae7628734ea0a5\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    uint256 hash;
    hash.SetHex(request.params[0].get_str());
    std::vector<uint256> vHash;
    vHash.push_back(hash);
    std::vector<uint256> vHashOut;

    if (pwallet->ZapSelectTx(vHash, vHashOut) != DB_LOAD_OK) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Could not properly delete the transaction.");
    }

    if(vHashOut.empty()) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Transaction does not exist in wallet.");
    }

    return NullUniValue;
}

UniValue importpubkey(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 4)
        throw std::runtime_error(
            "importpubkey \"pubkey\" ( \"label\" rescan )\n"
            "\nAdds a public key (in hex) that can be watched as if it were in your wallet but cannot be used to spend.\n"
            "\nArguments:\n"
            "1. \"pubkey\"           (string, required) The hex-encoded public key\n"
            "2. \"label\"            (string, optional, default=\"\") An optional label\n"
            "3. rescan               (boolean, optional, default=true) Rescan the wallet for transactions\n"
            "\nNote: This call can take minutes to complete if rescan is true.\n"
            "\nExamples:\n"
            "\nImport a public key with rescan\n"
            + HelpExampleCli("importpubkey", "\"mypubkey\"") +
            "\nImport using a label without rescan\n"
            + HelpExampleCli("importpubkey", "\"mypubkey\" \"testing\" false") +
            "\nAs a JSON-RPC call\n"
            + HelpExampleRpc("importpubkey", "\"mypubkey\", \"testing\", false")
        );

    //fixme: (Post-2.1)
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Sorry for the inconvenience, watch only addresses are temporarily disabled but will come back in a future release.");

    std::string strLabel = "";
    if (request.params.size() > 1)
        strLabel = request.params[1].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (request.params.size() > 2)
        fRescan = request.params[2].get_bool();

    if (fRescan && fPruneMode)
        throw JSONRPCError(RPC_WALLET_ERROR, "Rescan is disabled in pruned mode");

    if (!IsHex(request.params[0].get_str()))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Pubkey must be a hex string");
    std::vector<unsigned char> data(ParseHex(request.params[0].get_str()));
    CPubKey pubKey(data.begin(), data.end());
    if (!pubKey.IsFullyValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Pubkey is not a valid public key");

    LOCK2(cs_main, pwallet->cs_wallet);

    ImportAddress(pwallet, CGuldenAddress(pubKey.GetID()), strLabel);
    ImportScript(pwallet, GetScriptForRawPubKey(pubKey), strLabel, false);

    if (fRescan)
    {
        pwallet->ScanForWalletTransactions(chainActive.Genesis(), true);
        pwallet->ReacceptWalletTransactions();
    }

    return NullUniValue;
}


UniValue importwallet(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "importwallet \"filename\" \"account\"\n"
            "\nImports keys from a wallet dump file (see dumpwallet).\n"
            "\nArguments:\n"
            "1. \"filename\"    (string, required) The wallet file\n"
            "2. \"account\"     (string, required) The account in which to import the key. \"\" for the currently active account. NB! Must be a legacy account, cannot import keys into an HD account.\n"
            "\nExamples:\n"
            "\nDump the wallet\n"
            + HelpExampleCli("dumpwallet", "\"test\"") +
            "\nImport the wallet\n"
            + HelpExampleCli("importwallet", "\"test\"") +
            "\nImport using the json rpc call\n"
            + HelpExampleRpc("importwallet", "\"test\"")
        );

    if (fPruneMode)
        throw JSONRPCError(RPC_WALLET_ERROR, "Importing wallets is disabled in pruned mode");

    CAccount* forAccount = AccountFromValue(pwallet, request.params[1], true);
    LOCK2(cs_main, pwallet->cs_wallet);

    if (forAccount->IsHD())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot import a privkey into an HD account.");


    EnsureWalletIsUnlocked(pwallet);

    std::ifstream file;
    file.open(request.params[0].get_str().c_str(), std::ios::in | std::ios::ate);
    if (!file.is_open())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");

    int64_t nTimeBegin = chainActive.Tip()->GetBlockTime();

    bool fGood = true;

    int64_t nFilesize = std::max((int64_t)1, (int64_t)file.tellg());
    file.seekg(0, file.beg);

    pwallet->ShowProgress(_("Importing..."), 0); // show progress dialog in GUI
    while (file.good()) {
        pwallet->ShowProgress("", std::max(1, std::min(99, (int)(((double)file.tellg() / (double)nFilesize) * 100))));
        std::string line;
        std::getline(file, line);
        if (line.empty() || line[0] == '#')
            continue;

        std::vector<std::string> vstr;
        boost::split(vstr, line, boost::is_any_of(" "));
        if (vstr.size() < 2)
            continue;
        CGuldenSecret vchSecret;
        if (!vchSecret.SetString(vstr[0]))
            continue;
        CKey key = vchSecret.GetKey();
        CPubKey pubkey = key.GetPubKey();
        assert(key.VerifyPubKey(pubkey));
        CKeyID keyid = pubkey.GetID();
        if (pwallet->HaveKey(keyid)) {
            LogPrintf("Skipping import of %s (key already present)\n", CGuldenAddress(keyid).ToString());
            continue;
        }
        int64_t nTime = DecodeDumpTime(vstr[1]);
        std::string strLabel;
        bool fLabel = true;
        for (unsigned int nStr = 2; nStr < vstr.size(); nStr++) {
            if (boost::algorithm::starts_with(vstr[nStr], "#"))
                break;
            if (vstr[nStr] == "change=1")
                fLabel = false;
            if (vstr[nStr] == "reserve=1")
                fLabel = false;
            if (boost::algorithm::starts_with(vstr[nStr], "label=")) {
                strLabel = DecodeDumpString(vstr[nStr].substr(6));
                fLabel = true;
            }
        }
        LogPrintf("Importing %s...\n", CGuldenAddress(keyid).ToString());

        if (!pwallet->AddKeyPubKey(key, pubkey, *forAccount, KEYCHAIN_EXTERNAL)) {
            fGood = false;
            continue;
        }
        pwallet->mapKeyMetadata[keyid].nCreateTime = nTime;
        if (fLabel)
            pwallet->SetAddressBook(keyid.ToString(), strLabel, "receive");
        nTimeBegin = std::min(nTimeBegin, nTime);
    }
    file.close();
    pwallet->ShowProgress("", 100); // hide progress dialog in GUI
    pwallet->UpdateTimeFirstKey(nTimeBegin);

    CBlockIndex *pindex = chainActive.FindEarliestAtLeast(nTimeBegin - TIMESTAMP_WINDOW);

    LogPrintf("Rescanning last %i blocks\n", pindex ? chainActive.Height() - pindex->nHeight + 1 : 0);
    pwallet->ScanForWalletTransactions(pindex);
    pwallet->MarkDirty();

    if (!fGood)
        throw JSONRPCError(RPC_WALLET_ERROR, "Error adding some keys to wallet");

    return NullUniValue;
}

UniValue dumpprivkey(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "dumpprivkey \"address\"\n"
            "\nReveals the private key corresponding to 'address'.\n"
            "Then the importprivkey can be used with this output\n"
            "WARNING! If a dumped private key from an HD account is exposed or given out -all- keys within that account (current and future) are also at risk, if the attacker can also get hold of your public key for that account.\n"
            "\nArguments:\n"
            "1. \"address\"   (string, required) The Gulden address for the private key\n"
            "2. \"HDConsent\"        (string, optional) If dumping from an HD account please pass the string 'I_UNDERSTAND_AND_ACCEPT_THE_RISK_OF_DUMPING_AN_HD_PRIVKEY' for this paramater, if you do not understand the risk then please do not do this.\n"
            "\nResult:\n"
            "\"key\"                (string) The private key\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpprivkey", "\"myaddress\"")
            + HelpExampleCli("importprivkey", "\"mykey\"")
            + HelpExampleRpc("dumpprivkey", "\"myaddress\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    EnsureWalletIsUnlocked(pwallet);

    std::string strAddress = request.params[0].get_str();
    CGuldenAddress address;
    if (!address.SetString(strAddress))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Gulden address");
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    CKey vchSecret;

    for(const auto& accountIter : pwallet->mapAccounts)
    {
        if (accountIter.second->HaveKey(keyID))
        {
            if (accountIter.second->IsHD())
            {
                if (request.params.size() < 2 || request.params[1].get_str() != "I_UNDERSTAND_AND_ACCEPT_THE_RISK_OF_DUMPING_AN_HD_PRIVKEY")
                {
                    throw JSONRPCError(RPC_INTERNAL_ERROR, "Please pass the correct HDConsent option in order to proceed.");
                }
            }
        }
    }
    return CGuldenSecret(vchSecret).ToString();
}

UniValue dumpwallet(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "dumpwallet \"filename\"\n"
            "\nDumps all wallet keys in a human-readable format.\n"
            "WARNING! If a dumped private key from an HD account is exposed or given out -all- keys within that account (current and future) are also at risk, if the attacker can also get hold of your public key for that account.\n"
            "It is strongly advised not to use this function with an HD wallet, please proceed at your own risk.\n"
            "\nArguments:\n"
            "1. \"filename\"    (string, required) The filename with path (either absolute or relative to GuldenD)\n"
            "2. \"HDConsent\"        (string, optional) If dumping from an HD account please pass the string 'I_UNDERSTAND_AND_ACCEPT_THE_RISK_OF_DUMPING_AN_HD_PRIVKEY' for this paramater, if you do not understand the risk then please do not do this.\n"
            "\nResult:\n"
            "{                           (json object)\n"
            "  \"filename\" : {        (string) The filename with full absolute path\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpwallet", "\"test\"")
            + HelpExampleRpc("dumpwallet", "\"test\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    EnsureWalletIsUnlocked(pwallet);

    std::ofstream file;
    boost::filesystem::path filepath = request.params[0].get_str();
    filepath = boost::filesystem::absolute(filepath);
    file.open(filepath.string().c_str());
    if (!file.is_open())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");

    for(const auto& accountIter : pwallet->mapAccounts)
    {
        if (accountIter.second->IsHD())
        {
            if (request.params.size() < 2 || request.params[1].get_str() != "I_UNDERSTAND_AND_ACCEPT_THE_RISK_OF_DUMPING_AN_HD_PRIVKEY")
            {
                throw JSONRPCError(RPC_INTERNAL_ERROR, "Please pass the correct HDConsent option in order to proceed.");
            }
        }
    }

    std::map<CKeyID, int64_t> mapKeyBirth;
    std::set<CKeyID> setKeyPool;
    pwallet->GetKeyBirthTimes(mapKeyBirth);
    pwallet->GetAllReserveKeys(setKeyPool);

    // sort time/key pairs
    std::vector<std::pair<int64_t, CKeyID> > vKeyBirth;
    for (std::map<CKeyID, int64_t>::const_iterator it = mapKeyBirth.begin(); it != mapKeyBirth.end(); it++) {
        vKeyBirth.push_back(std::make_pair(it->second, it->first));
    }
    mapKeyBirth.clear();
    std::sort(vKeyBirth.begin(), vKeyBirth.end());

    // produce output
    file << strprintf("# Wallet dump created by Gulden %s\n", CLIENT_BUILD);
    file << strprintf("# * Created on %s\n", EncodeDumpTime(GetTime()));
    file << strprintf("# * Best block at time of backup was %i (%s),\n", chainActive.Height(), chainActive.Tip()->GetBlockHashPoW2().ToString());
    file << strprintf("#   mined on %s\n", EncodeDumpTime(chainActive.Tip()->GetBlockTime()));
    file << "\n";

    // add the base58check encoded extended master if the wallet uses HD 
    /*GULDEN - no masterkeyid...
    CKeyID masterKeyID = pwallet->GetHDChain().masterKeyID;
    if (!masterKeyID.IsNull())
    {
        CKey key;
        if (pwallet->GetKey(masterKeyID, key)) {
            CExtKey masterKey;
            masterKey.SetMaster(key.begin(), key.size());

            CGuldenExtKey b58extkey;
            b58extkey.SetKey(masterKey);

            file << "# extended private masterkey: " << b58extkey.ToString() << "\n\n";
        }
    }*/
    for (std::vector<std::pair<int64_t, CKeyID> >::const_iterator it = vKeyBirth.begin(); it != vKeyBirth.end(); it++) {
        const CKeyID &keyid = it->second;
        std::string strTime = EncodeDumpTime(it->first);
        std::string strAddr = CGuldenAddress(keyid).ToString();
        CKey key;
        if (pwallet->GetKey(keyid, key)) {
            file << strprintf("%s %s ", CGuldenSecret(key).ToString(), strTime);
            if (pwallet->mapAddressBook.count(CGuldenAddress(keyid).ToString())) {
                file << strprintf("label=%s", EncodeDumpString(pwallet->mapAddressBook[CGuldenAddress(keyid).ToString()].name));
            /*} else if (keyid == masterKeyID) {
                file << "hdmaster=1";*/
            } else if (setKeyPool.count(keyid)) {
                file << "reserve=1";
            /*} else if (pwallet->mapKeyMetadata[keyid].hdKeypath == "m") {
                file << "inactivehdmaster=1";*/
            } else {
                file << "change=1";
            }
            file << strprintf(" # addr=%s%s\n", strAddr, (pwallet->mapKeyMetadata[keyid].hdKeypath.size() > 0 ? " hdkeypath="+pwallet->mapKeyMetadata[keyid].hdKeypath : ""));
        }
    }
    file << "\n";
    file << "# End of dump\n";
    file.close();

    UniValue reply(UniValue::VOBJ);
    reply.push_back(Pair("filename", filepath.string()));

    return reply;
}


static UniValue ProcessImport(CWallet* const pwallet, CAccount* forAccount, const UniValue& data, const int64_t timestamp)
{
    try {
        bool success = false;

        // Required fields.
        const UniValue& scriptPubKey = data["scriptPubKey"];

        // Should have script or JSON with "address".
        if (!(scriptPubKey.getType() == UniValue::VOBJ && scriptPubKey.exists("address")) && !(scriptPubKey.getType() == UniValue::VSTR)) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid scriptPubKey");
        }

        // Optional fields.
        const std::string& strRedeemScript = data.exists("redeemscript") ? data["redeemscript"].get_str() : "";
        const UniValue& pubKeys = data.exists("pubkeys") ? data["pubkeys"].get_array() : UniValue();
        const UniValue& keys = data.exists("keys") ? data["keys"].get_array() : UniValue();
        const bool& internal = data.exists("internal") ? data["internal"].get_bool() : false;
        const bool& watchOnly = data.exists("watchonly") ? data["watchonly"].get_bool() : false;
        const std::string& label = data.exists("label") && !internal ? data["label"].get_str() : "";
        const int64_t& timestamp = data.exists("timestamp") && data["timestamp"].get_int64() > 1 ? data["timestamp"].get_int64() : 1;

        bool isScript = scriptPubKey.getType() == UniValue::VSTR;
        bool isP2SH = strRedeemScript.length() > 0;
        const std::string& output = isScript ? scriptPubKey.get_str() : scriptPubKey["address"].get_str();

        // Parse the output.
        CScript script;
        CGuldenAddress address;

        if (!isScript) {
            address = CGuldenAddress(output);
            if (!address.IsValid()) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            }
            script = GetScriptForDestination(address.Get());
        } else {
            if (!IsHex(output)) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid scriptPubKey");
            }

            std::vector<unsigned char> vData(ParseHex(output));
            script = CScript(vData.begin(), vData.end());
        }

        // Watchonly and private keys
        if (watchOnly && keys.size()) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Incompatibility found between watchonly and keys");
        }

        // Internal + Label
        if (internal && data.exists("label")) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Incompatibility found between internal and label");
        }

        // Not having Internal + Script
        if (!internal && isScript) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Internal must be set for hex scriptPubKey");
        }

        // Keys / PubKeys size check.
        if (!isP2SH && (keys.size() > 1 || pubKeys.size() > 1)) { // Address / scriptPubKey
            throw JSONRPCError(RPC_INVALID_PARAMETER, "More than private key given for one address");
        }

        // Invalid P2SH redeemScript
        if (isP2SH && !IsHex(strRedeemScript)) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid redeem script");
        }

        // Process. //

        // P2SH
        if (isP2SH)
        {
            // Import redeem script.
            std::vector<unsigned char> vData(ParseHex(strRedeemScript));
            CScript redeemScript = CScript(vData.begin(), vData.end());

            // Invalid P2SH address
            if (!script.IsPayToScriptHash()) {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid P2SH address / script");
            }

            pwallet->MarkDirty();

            //fixme: (WATCHONLY) - timestamp
            //fixme: (Post-2.1) (REIMPLEMENT AS SPECIAL WATCH ACCOUNT)
            //if (!forAccount->AddWatchOnly(redeemScript/*, timestamp*/)) {
                //throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
            //}

            if (!forAccount->HaveCScript(redeemScript) && !forAccount->AddCScript(redeemScript)) {
                throw JSONRPCError(RPC_WALLET_ERROR, "Error adding p2sh redeemScript to wallet");
            }

            CGuldenAddress redeemAddress = CGuldenAddress(CScriptID(redeemScript));
            CScript redeemDestination = GetScriptForDestination(redeemAddress.Get());

            if (::IsMine(*forAccount, redeemDestination) == ISMINE_SPENDABLE) {
                throw JSONRPCError(RPC_WALLET_ERROR, "The wallet already contains the private key for this address or script");
            }

            pwallet->MarkDirty();

            //fixme: (Post-2.1) (REIMPLEMENT AS SPECIAL WATCH ACCOUNT)
            //fixme: (WATCHONLY) - timestamp
            //if (!forAccount->AddWatchOnly(redeemDestination/*, timestamp*/)) {
                //throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
            //}

            // add to address book or update label
            if (address.IsValid()) {
                pwallet->SetAddressBook(address.ToString(), label, "receive");
            }

            // Import private keys.
            if (keys.size()) {
                for (size_t i = 0; i < keys.size(); i++) {
                    const std::string& privkey = keys[i].get_str();

                    CGuldenSecret vchSecret;
                    bool fGood = vchSecret.SetString(privkey);

                    if (!fGood) {
                        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key encoding");
                    }

                    CKey key = vchSecret.GetKey();

                    if (!key.IsValid()) {
                        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");
                    }

                    CPubKey pubkey = key.GetPubKey();
                    assert(key.VerifyPubKey(pubkey));

                    CKeyID vchAddress = pubkey.GetID();
                    pwallet->MarkDirty();
                    pwallet->SetAddressBook(CGuldenAddress(vchAddress).ToString(), label, "receive");

                    if (forAccount->HaveKey(vchAddress)) {
                        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Already have this key");
                    }

                    pwallet->mapKeyMetadata[vchAddress].nCreateTime = timestamp;

                    if (!forAccount->AddKeyPubKey(key, pubkey)) {
                        throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");
                    }

                    pwallet->UpdateTimeFirstKey(timestamp);
                }
            }

            success = true;
        }
        else
        {
            // Import public keys.
            if (pubKeys.size() && keys.size() == 0)
            {
                const std::string& strPubKey = pubKeys[0].get_str();

                if (!IsHex(strPubKey)) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Pubkey must be a hex string");
                }

                std::vector<unsigned char> vData(ParseHex(strPubKey));
                CPubKey pubKey(vData.begin(), vData.end());

                if (!pubKey.IsFullyValid()) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Pubkey is not a valid public key");
                }

                CGuldenAddress pubKeyAddress = CGuldenAddress(pubKey.GetID());

                // Consistency check.
                if (!isScript && !(pubKeyAddress.Get() == address.Get())) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Consistency check failed");
                }

                // Consistency check.
                if (isScript) {
                    CGuldenAddress scriptAddress;
                    CTxDestination destination;

                    if (ExtractDestination(script, destination)) {
                        scriptAddress = CGuldenAddress(destination);
                        if (!(scriptAddress.Get() == pubKeyAddress.Get())) {
                            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Consistency check failed");
                        }
                    }
                }

                CScript pubKeyScript = GetScriptForDestination(pubKeyAddress.Get());

                if (::IsMine(*forAccount, pubKeyScript) == ISMINE_SPENDABLE) {
                    throw JSONRPCError(RPC_WALLET_ERROR, "The wallet already contains the private key for this address or script");
                }

                pwallet->MarkDirty();

                //fixme: (Post-2.1) (REIMPLEMENT AS SPECIAL WATCH ACCOUNT)
                //fixme: (WATCHONLY) - timestamp
                //if (!forAccount->AddWatchOnly(pubKeyScript/*, timestamp*/)) {
                    //throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
                //}

                // add to address book or update label
                if (pubKeyAddress.IsValid()) {
                    pwallet->SetAddressBook(pubKeyAddress.ToString(), label, "receive");
                }

                // TODO Is this necessary?
                CScript scriptRawPubKey = GetScriptForRawPubKey(pubKey);

                if (::IsMine(*forAccount, scriptRawPubKey) == ISMINE_SPENDABLE) {
                    throw JSONRPCError(RPC_WALLET_ERROR, "The wallet already contains the private key for this address or script");
                }

                pwallet->MarkDirty();

                //fixme: (Post-2.1) (REIMPLEMENT AS SPECIAL WATCH ACCOUNT)
                //fixme: (WATCHONLY) - timestamp
                //if (!forAccount->AddWatchOnly(scriptRawPubKey/*, timestamp*/)) {
                    //throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
                //}

                success = true;
            }

            // Import private keys.
            if (keys.size())
            {
                const std::string& strPrivkey = keys[0].get_str();

                // Checks.
                CGuldenSecret vchSecret;
                bool fGood = vchSecret.SetString(strPrivkey);

                if (!fGood) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key encoding");
                }

                CKey key = vchSecret.GetKey();
                if (!key.IsValid()) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");
                }

                CPubKey pubKey = key.GetPubKey();
                assert(key.VerifyPubKey(pubKey));

                CGuldenAddress pubKeyAddress = CGuldenAddress(pubKey.GetID());

                // Consistency check.
                if (!isScript && !(pubKeyAddress.Get() == address.Get())) {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Consistency check failed");
                }

                // Consistency check.
                if (isScript) {
                    CGuldenAddress scriptAddress;
                    CTxDestination destination;

                    if (ExtractDestination(script, destination)) {
                        scriptAddress = CGuldenAddress(destination);
                        if (!(scriptAddress.Get() == pubKeyAddress.Get())) {
                            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Consistency check failed");
                        }
                    }
                }

                CKeyID vchAddress = pubKey.GetID();
                pwallet->MarkDirty();
                pwallet->SetAddressBook(CGuldenAddress(vchAddress).ToString(), label, "receive");

                if (forAccount->HaveKey(vchAddress)) {
                    return false;
                }

                pwallet->mapKeyMetadata[vchAddress].nCreateTime = timestamp;

                if (!forAccount->AddKeyPubKey(key, pubKey)) {
                    throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");
                }

                pwallet->UpdateTimeFirstKey(timestamp);

                success = true;
            }

            // Import scriptPubKey only.
            if (pubKeys.size() == 0 && keys.size() == 0)
            {
                if (::IsMine(*forAccount, script) == ISMINE_SPENDABLE) {
                    throw JSONRPCError(RPC_WALLET_ERROR, "The wallet already contains the private key for this address or script");
                }

                pwallet->MarkDirty();

                //fixme: (Post-2.1) (REIMPLEMENT AS SPECIAL WATCH ACCOUNT)
                //fixme: (WATCHONLY) - timestamp
                //if (!forAccount->AddWatchOnly(script/*, timestamp*/)) {
                    //throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
                //}

                if (scriptPubKey.getType() == UniValue::VOBJ) {
                    // add to address book or update label
                    if (address.IsValid()) {
                        pwallet->SetAddressBook(address.ToString(), label, "receive");
                    }
                }

                success = true;
            }
        }

        UniValue result = UniValue(UniValue::VOBJ);
        result.pushKV("success", UniValue(success));
        return result;
    } catch (const UniValue& e) {
        UniValue result = UniValue(UniValue::VOBJ);
        result.pushKV("success", UniValue(false));
        result.pushKV("error", e);
        return result;
    } catch (...) {
        UniValue result = UniValue(UniValue::VOBJ);
        result.pushKV("success", UniValue(false));
        result.pushKV("error", JSONRPCError(RPC_MISC_ERROR, "Missing required fields"));
        return result;
    }
}

static int64_t GetImportTimestamp(const UniValue& data, int64_t now)
{
    if (data.exists("timestamp")) {
        const UniValue& timestamp = data["timestamp"];
        if (timestamp.isNum()) {
            return timestamp.get_int64();
        } else if (timestamp.isStr() && timestamp.get_str() == "now") {
            return now;
        }
        throw JSONRPCError(RPC_TYPE_ERROR, strprintf("Expected number or \"now\" timestamp value for key. got type %s", uvTypeName(timestamp.type())));
    }
    throw JSONRPCError(RPC_TYPE_ERROR, "Missing required timestamp field for key");
}

UniValue importmulti(const JSONRPCRequest& mainRequest)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(mainRequest);
    if (!EnsureWalletIsAvailable(pwallet, mainRequest.fHelp)) {
        return NullUniValue;
    }

    if (mainRequest.fHelp || mainRequest.params.size() < 2 || mainRequest.params.size() > 3)
        throw std::runtime_error(
            "importmulti \"account\" \"requests\" ( \"options\" )\n\n"
            "Import addresses/scripts (with private or public keys, redeem script (P2SH)), rescanning all addresses in one-shot-only (rescan can be disabled via options). Requires a new wallet backup.\n\n"
            "Arguments:\n"
            "1. \"account\"  (string, required) The account in which to import the requests. \"\" for the currently active account. NB! Must be a legacy account, cannot import keys into an HD account.\n"
            "2. requests     (array, required) Data to be imported\n"
            "  [     (array of json objects)\n"
            "    {\n"
            "      \"scriptPubKey\": \"<script>\" | { \"address\":\"<address>\" }, (string / json, required) Type of scriptPubKey (string for script, json for address)\n"
            "      \"timestamp\": timestamp | \"now\"                            , (integer / string, required) Creation time of the key in seconds since epoch (Jan 1 1970 GMT),\n"
            "                                                                    or the string \"now\" to substitute the current synced blockchain time. The timestamp of the oldest\n"
            "                                                                    key will determine how far back blockchain rescans need to begin for missing wallet transactions.\n"
            "                                                                    \"now\" can be specified to bypass scanning, for keys which are known to never have been used, and\n"
            "                                                                    0 can be specified to scan the entire blockchain. Blocks up to 2 hours before the earliest key\n"
            "                                                                    creation time of all keys being imported by the importmulti call will be scanned.\n"
            "      \"redeemscript\": \"<script>\"                                , (string, optional) Allowed only if the scriptPubKey is a P2SH address or a P2SH scriptPubKey\n"
            "      \"pubkeys\": [\"<pubKey>\", ... ]                             , (array, optional) Array of strings giving pubkeys that must occur in the output or redeemscript\n"
            "      \"keys\": [\"<key>\", ... ]                                   , (array, optional) Array of strings giving private keys whose corresponding public keys must occur in the output or redeemscript\n"
            "      \"internal\": <true>                                          , (boolean, optional, default: false) Stating whether matching outputs should be treated as not incoming payments\n"
            "      \"watchonly\": <true>                                         , (boolean, optional, default: false) Stating whether matching outputs should be considered watched even when they're not spendable, only allowed if keys are empty\n"
            "      \"label\": <label>                                            , (string, optional, default: '') Label to assign to the address (aka account name, for now), only allowed with internal=false\n"
            "    }\n"
            "  ,...\n"
            "  ]\n"
            "3. options                 (json, optional)\n"
            "  {\n"
            "     \"rescan\": <false>,         (boolean, optional, default: true) Stating if should rescan the blockchain after all imports\n"
            "  }\n"
            "\nNote: This call can take minutes to complete if rescan is true, during that time, other rpc calls\n"
            "may report that the imported keys, addresses or scripts exists but related transactions are still missing.\n"
            "\nExamples:\n" +
            HelpExampleCli("importmulti", "'[{ \"scriptPubKey\": { \"address\": \"<my address>\" }, \"timestamp\":1455191478 }, "
                                          "{ \"scriptPubKey\": { \"address\": \"<my 2nd address>\" }, \"label\": \"example 2\", \"timestamp\": 1455191480 }]'") +
            HelpExampleCli("importmulti", "'[{ \"scriptPubKey\": { \"address\": \"<my address>\" }, \"timestamp\":1455191478 }]' '{ \"rescan\": false}'") +

            "\nResponse is an array with the same size as the input that has the execution result :\n"
            "  [{ \"success\": true } , { \"success\": false, \"error\": { \"code\": -1, \"message\": \"Internal Server Error\"} }, ... ]\n");

    RPCTypeCheck(mainRequest.params, {UniValue::VSTR, UniValue::VARR, UniValue::VOBJ});

    CAccount* forAccount = AccountFromValue(pwallet, mainRequest.params[0], true);
    if (forAccount->IsHD())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot import a privkey into an HD account.");

    const UniValue& requests = mainRequest.params[1];

    //Default options
    bool fRescan = true;

    if (!mainRequest.params[2].isNull())
    {
        const UniValue& options = mainRequest.params[2];
        if (options.exists("rescan"))
            fRescan = options["rescan"].get_bool();
    }

    LOCK2(cs_main, pwallet->cs_wallet);
    EnsureWalletIsUnlocked(pwallet);

    int64_t now = 0;
    bool fRunScan = false;
    int64_t nLowestTimestamp = 0;
    UniValue response(UniValue::VARR);
    {
        // Verify all timestamps are present before importing any keys.
        now = chainActive.Tip() ? chainActive.Tip()->GetMedianTimePast() : 0;
        for (const UniValue& data : requests.getValues())
        {
            GetImportTimestamp(data, now);
        }

        const int64_t minimumTimestamp = 1;
        if (fRescan && chainActive.Tip())
            nLowestTimestamp = chainActive.Tip()->GetBlockTime();
        else
            fRescan = false;

        for (const UniValue& data : requests.getValues())
        {
            const int64_t timestamp = std::max(GetImportTimestamp(data, now), minimumTimestamp);
            const UniValue result = ProcessImport(pwallet, forAccount, data, timestamp);
            response.push_back(result);

            if (!fRescan)
                continue;

            if (result["success"].get_bool())// If at least one request was successful then allow rescan.
                fRunScan = true;

            if (timestamp < nLowestTimestamp)// Get the lowest timestamp.
                nLowestTimestamp = timestamp;
        }
    }
    if (fRescan && fRunScan && requests.size())
    {
        int64_t scannedTime = pwallet->RescanFromTime(nLowestTimestamp, true /* update */);
        pwallet->ReacceptWalletTransactions();

        if (pwallet->IsAbortingRescan()) { throw JSONRPCError(RPC_MISC_ERROR, "Rescan aborted by user."); }
        if (scannedTime > nLowestTimestamp)
        {
            std::vector<UniValue> results = response.getValues();
            response.clear();
            response.setArray();
            size_t i = 0;
            for (const UniValue& request : requests.getValues()) {
                // If key creation date is within the successfully scanned
                // range, or if the import result already has an error set, let
                // the result stand unmodified. Otherwise replace the result
                // with an error message.
                if (scannedTime <= GetImportTimestamp(request, now) || results.at(i).exists("error"))
                {
                    response.push_back(results.at(i));
                }
                else
                {
                    UniValue result = UniValue(UniValue::VOBJ);
                    result.pushKV("success", UniValue(false));
                    std::string errorMessage = strprintf("Rescan failed for key with creation timestamp %d. There was an error reading a block from time %d, which is after or within %d seconds of key creation, and "
                                      "could contain transactions pertaining to the key. As a result, transactions and coins using this key may not appear in the wallet. This error could be "
                                      "caused by pruning or data corruption (see Gulden log for details) and could be dealt with by downloading and rescanning the relevant blocks (see -reindex and -rescan options).", GetImportTimestamp(request, now), scannedTime - TIMESTAMP_WINDOW - 1, TIMESTAMP_WINDOW);
                    result.pushKV("error", JSONRPCError(RPC_MISC_ERROR,errorMessage));
                    response.push_back(std::move(result));
                }
                ++i;
            }
        }
    }

    return response;
}
