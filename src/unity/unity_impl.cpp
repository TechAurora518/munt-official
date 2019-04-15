// Copyright (c) 2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

// Unity specific includes
#include "unity_impl.h"
#include "libinit.h"

// Standard gulden headers
#include "util.h"
#include "Gulden/util.h"
#include "ui_interface.h"
#include "wallet/wallet.h"
#include "unity/appmanager.h"
#include "utilmoneystr.h"
#include <chain.h>
#include "consensus/validation.h"
#include "net.h"
#include "Gulden/mnemonic.h"
#include "net_processing.h"
#include "wallet/spvscanner.h"
#include "sync.h"

// Djinni generated files
#include "gulden_unified_backend.hpp"
#include "gulden_unified_frontend.hpp"
#include "qr_code_record.hpp"
#include "balance_record.hpp"
#include "uri_record.hpp"
#include "uri_recipient.hpp"
#include "mutation_record.hpp"
#include "transaction_record.hpp"
#include "input_record.hpp"
#include "output_record.hpp"
#include "address_record.hpp"
#include "peer_record.hpp"
#include "block_info_record.hpp"
#include "monitor_record.hpp"
#include "gulden_monitor_listener.hpp"
#include "payment_result_status.hpp"
#ifdef __ANDROID__
#include "djinni_support.hpp"
#endif

// External libraries
#include <boost/algorithm/string.hpp>
#include <boost/program_options/parsers.hpp>
#include <qrencode.h>
#include <memory>

std::shared_ptr<GuldenUnifiedFrontend> signalHandler;

CCriticalSection cs_monitoringListeners;
std::set<std::shared_ptr<GuldenMonitorListener> > monitoringListeners;

const int RECOMMENDED_CONFIRMATIONS = 3;

TransactionStatus getStatusForTransaction(const CWalletTx* wtx)
{
    TransactionStatus status;
    int depth = wtx->GetDepthInMainChain();
    if (depth < 0)
        status = TransactionStatus::CONFLICTED;
    else if (depth == 0) {
        if (wtx->isAbandoned())
            status = TransactionStatus::ABANDONED;
        else
            status = TransactionStatus::UNCONFIRMED;
    }
    else if (depth < RECOMMENDED_CONFIRMATIONS) {
        status = TransactionStatus::CONFIRMING;
    }
    else {
        status = TransactionStatus::CONFIRMED;
    }
    return status;
}

void addMutationsForTransaction(const CWalletTx* wtx, std::vector<MutationRecord>& mutations)
{
    int64_t subtracted = wtx->GetDebit(ISMINE_SPENDABLE, pactiveWallet->activeAccount, true);
    int64_t added = wtx->GetCredit(ISMINE_SPENDABLE, pactiveWallet->activeAccount, true);

    uint64_t time = wtx->nTimeSmart;
    std::string hash = wtx->GetHash().ToString();

    TransactionStatus status = getStatusForTransaction(wtx);
    int depth = wtx->GetDepthInMainChain();

    // if any funds were subtracted the transaction was sent by us
    if (subtracted > 0)
    {
        int64_t fee = subtracted - wtx->tx->GetValueOut();
        int64_t change = wtx->GetChange();

        // detect internal transfer and split it
        if (subtracted - fee == added)
        {
            // amount received
            mutations.push_back(MutationRecord(added - change, time, hash, status, depth));

            // amount send including fee
            mutations.push_back(MutationRecord(change - subtracted, time, hash, status, depth));
        }
        else
        {
            mutations.push_back(MutationRecord(added - subtracted, time, hash, status, depth));
        }
    }
    else if (added != 0) // nothing subtracted so we received funds
    {
        mutations.push_back(MutationRecord(added, time, hash, status, depth));
    }
}

TransactionRecord calculateTransactionRecordForWalletTransaction(const CWalletTx& wtx)
{
    CWallet* pwallet = pactiveWallet;

    std::vector<InputRecord> inputs;
    std::vector<OutputRecord> outputs;

    int64_t subtracted = wtx.GetDebit(ISMINE_SPENDABLE, pactiveWallet->activeAccount, true);
    int64_t added = wtx.GetCredit(ISMINE_SPENDABLE, pactiveWallet->activeAccount, true);

    CAmount fee = 0;
    // if any funds were subtracted the transaction was sent by us
    if (subtracted > 0)
        fee = subtracted - wtx.tx->GetValueOut();

    const CTransaction& tx = *wtx.tx;

    for (const CTxIn& txin: tx.vin) {
        std::string address;
        CGuldenAddress addr;
        CTxDestination dest = CNoDestination();

        // Try to extract destination, this is not possible in general. Only if the previous
        // ouput of our input happens to be in our wallet. Which will usually only be the case for
        // our own transactions.
        std::map<uint256, CWalletTx>::const_iterator mi = pwallet->mapWallet.find(txin.prevout.getHash());
        if (mi != pwallet->mapWallet.end())
        {
            const CWalletTx& prev = (*mi).second;
            if (txin.prevout.n < prev.tx->vout.size())
            {
                const auto& prevOut =  prev.tx->vout[txin.prevout.n];
                if (!ExtractDestination(prevOut, dest) && !prevOut.IsUnspendable())
                {
                    LogPrintf("Unknown transaction type found, txid %s\n", wtx.GetHash().ToString());
                    dest = CNoDestination();
                }
            }
        }
        if (addr.Set(dest))
            address = addr.ToString();
        std::string label;
        if (pwallet->mapAddressBook.count(address))
            label = pwallet->mapAddressBook[address].name;
        inputs.push_back(InputRecord(address, label, static_cast<const CGuldenWallet*>(pwallet)->IsMine(*pactiveWallet->activeAccount, txin)));
    }

    for (const CTxOut& txout: tx.vout) {
        std::string address;
        CGuldenAddress addr;
        CTxDestination dest;
        if (!ExtractDestination(txout, dest) && !txout.IsUnspendable())
        {
            LogPrintf("Unknown transaction type found, txid %s\n", tx.GetHash().ToString());
            dest = CNoDestination();
        }

        if (addr.Set(dest))
            address = addr.ToString();
        std::string label;
        if (pwallet->mapAddressBook.count(address))
            label = pwallet->mapAddressBook[address].name;
        outputs.push_back(OutputRecord(txout.nValue, address, label, IsMine(*pactiveWallet->activeAccount, txout)));
    }

    TransactionStatus status = getStatusForTransaction(&wtx);

    return TransactionRecord(wtx.GetHash().ToString(), wtx.nTimeSmart,
                             added - subtracted,
                             fee, status, wtx.nHeight, wtx.nBlockTime, wtx.GetDepthInMainChain(),
                             inputs, outputs);
}

static void notifyBalanceChanged(CWallet* pwallet)
{
    if (pwallet && signalHandler)
    {
        WalletBalances balances;
        pwallet->GetBalances(balances, pactiveWallet->activeAccount, true);
        signalHandler->notifyBalanceChange(BalanceRecord(balances.availableIncludingLocked, balances.availableExcludingLocked, balances.availableLocked, balances.unconfirmedIncludingLocked, balances.unconfirmedExcludingLocked, balances.unconfirmedLocked, balances.immatureIncludingLocked, balances.immatureExcludingLocked, balances.immatureLocked, balances.totalLocked));
    }
}

void terminateUnityFrontend()
{
    if (signalHandler)
    {
        signalHandler->notifyShutdown();
    }
}

void handlePostInitMain()
{
    if (signalHandler)
    {
        signalHandler->notifyCoreReady();
    }

    // unified progress notification
    uiInterface.NotifyUnifiedProgress.connect([=](float progress) {
        if (signalHandler)
            signalHandler->notifyUnifiedProgress(progress);
    });

    // monitoring listeners notifications
    uiInterface.NotifyHeaderProgress.connect([=](int, int, int, int64_t) {
        int32_t height, probable_height, offset;
        {
            LOCK(cs_main);
            height = partialChain.Height();
            probable_height = GetProbableHeight();
            offset = partialChain.HeightOffset();
        }
        LOCK(cs_monitoringListeners);
        for (const auto &listener: monitoringListeners) {
            listener->onPartialChain(height, probable_height, offset);
        }
    });

    uiInterface.NotifySPVPrune.connect([=](int height) {
        LOCK(cs_monitoringListeners);
        for (const auto &listener: monitoringListeners) {
            listener->onPruned(height);
        }
    });

    uiInterface.NotifySPVProgress.connect([=](int /*start_height*/, int processed_height, int /*probable_height*/) {
        LOCK(cs_monitoringListeners);
        for (const auto &listener: monitoringListeners) {
            listener->onProcessedSPVBlocks(processed_height);
        }
    });

    // Update transaction/balance changes
    if (pactiveWallet)
    {
        // Fire events for transaction depth changes (up to depth 10 only)
        pactiveWallet->NotifyTransactionDepthChanged.connect( [&](CWallet* pwallet, const uint256& hash)
        {
            DS_LOCK2(cs_main, pwallet->cs_wallet);
            if (pwallet->mapWallet.find(hash) != pwallet->mapWallet.end())
            {
                const CWalletTx& wtx = pwallet->mapWallet[hash];
                LogPrintf("unity: notify transaction depth changed %s",hash.ToString().c_str());
                if (signalHandler)
                {
                    TransactionRecord walletTransaction = calculateTransactionRecordForWalletTransaction(wtx);
                    signalHandler->notifyUpdatedTransaction(walletTransaction);
                }
            }
        } );
        // Fire events for transaction status changes, or new transactions (this won't fire for simple depth changes)
        pactiveWallet->NotifyTransactionChanged.connect( [&](CWallet* pwallet, const uint256& hash, ChangeType status, bool fSelfComitted)
        {
            {
                DS_LOCK2(cs_main, pwallet->cs_wallet);
                if (pwallet->mapWallet.find(hash) != pwallet->mapWallet.end())
                {
                    const CWalletTx& wtx = pwallet->mapWallet[hash];
                    if (status == CT_NEW || status == CT_UPDATED)
                    {
                        LogPrintf("unity: notify transaction changed [2] %s",hash.ToString().c_str());
                        if (signalHandler)
                        {
                            if (status == CT_NEW) {
                                std::vector<MutationRecord> mutations;
                                addMutationsForTransaction(&wtx, mutations);
                                for (auto& m: mutations) {
                                    signalHandler->notifyNewMutation(m, fSelfComitted);
                                }
                            }
                            else { // status == CT_UPDATED
                                TransactionRecord walletTransaction = calculateTransactionRecordForWalletTransaction(wtx);
                                signalHandler->notifyUpdatedTransaction(walletTransaction);
                            }
                        }
                    }
                    else if (status == CT_DELETED)
                    {
                        //fixme: (UNITY) - Consider implementing f.e.x if a 0 conf transaction gets deleted...
                    }
                }
            }
            notifyBalanceChanged(pwallet);
        } );

        // Fire once immediately to update with latest on load.
        notifyBalanceChanged(pactiveWallet);
    }
}

void handleInitWithExistingWallet()
{
    if (signalHandler)
    {
        signalHandler->notifyInitWithExistingWallet();
    }
    GuldenAppManager::gApp->initialize();
}

void handleInitWithoutExistingWallet()
{
    signalHandler->notifyInitWithoutExistingWallet();
}

bool GuldenUnifiedBackend::InitWalletFromRecoveryPhrase(const std::string& phrase, const std::string& password)
{
    // Refuse to acknowledge an empty recovery phrase, or one that doesn't pass even the most obvious requirement
    if (phrase.length() < 16)
    {
        return false;
    }

    //fixme: (SPV) - Handle all the various birth date (or lack of birthdate) cases here instead of just the one.
    SecureString phraseOnly;
    int phraseBirthNumber = 0;
    GuldenAppManager::gApp->splitRecoveryPhraseAndBirth(phrase.c_str(), phraseOnly, phraseBirthNumber);
    if (!checkMnemonic(phraseOnly))
    {
        return false;
    }

    //fixme: (SPV) - Handle all the various birth date (or lack of birthdate) cases here instead of just the one.
    GuldenAppManager::gApp->setRecoveryPhrase(phraseOnly);
    GuldenAppManager::gApp->setRecoveryBirthNumber(phraseBirthNumber);
    GuldenAppManager::gApp->setRecoveryPassword(password.c_str());
    GuldenAppManager::gApp->isRecovery = true;
    GuldenAppManager::gApp->initialize();

    return true;
}

bool ValidateAndSplitRecoveryPhrase(const std::string & phrase, SecureString& mnemonic, int& birthNumber)
{
    if (phrase.length() < 16)
        return false;

    GuldenAppManager::gApp->splitRecoveryPhraseAndBirth(phrase.c_str(), mnemonic, birthNumber);
    return checkMnemonic(mnemonic) && (birthNumber == 0 || Base10ChecksumDecode(birthNumber, nullptr));
}

bool GuldenUnifiedBackend::ContinueWalletFromRecoveryPhrase(const std::string& phrase, const std::string& password)
{
    SecureString phraseOnly;
    int phraseBirthNumber;

    if (!ValidateAndSplitRecoveryPhrase(phrase, phraseOnly, phraseBirthNumber))
        return false;

    if (!pactiveWallet)
    {
        LogPrintf("ContineWalletFromRecoveryPhrase: No active wallet");
        return false;
    }

    LOCK2(cs_main, pactiveWallet->cs_wallet);
    GuldenAppManager::gApp->setRecoveryPhrase(phraseOnly);
    GuldenAppManager::gApp->setRecoveryBirthNumber(phraseBirthNumber);
    GuldenAppManager::gApp->setRecoveryPassword(password.c_str());
    GuldenAppManager::gApp->isRecovery = true;

    CWallet::CreateSeedAndAccountFromPhrase(pactiveWallet);

    // Allow update of balance for deleted accounts/transactions
    LogPrintf("%s: Update balance and rescan", __func__);
    notifyBalanceChanged(pactiveWallet);

    // Rescan for transactions on the linked account
    DoRescan();

    return true;
}

bool GuldenUnifiedBackend::IsValidRecoveryPhrase(const std::string & phrase)
{
    SecureString dummyMnemonic;
    int dummyNumber;
    return ValidateAndSplitRecoveryPhrase(phrase, dummyMnemonic, dummyNumber);
}

std::string GuldenUnifiedBackend::GenerateRecoveryMnemonic()
{
    std::vector<unsigned char> entropy(16);
    GetStrongRandBytes(&entropy[0], 16);
    return GuldenAppManager::gApp->composeRecoveryPhrase(mnemonicFromEntropy(entropy, entropy.size()*8), GetAdjustedTime()).c_str();
}

std::string GuldenUnifiedBackend::ComposeRecoveryPhrase(const std::string & mnemonic, int64_t birthTime)
{
    return std::string(GuldenAppManager::composeRecoveryPhrase(SecureString(mnemonic), birthTime));
}

bool GuldenUnifiedBackend::InitWalletLinkedFromURI(const std::string& linked_uri, const std::string& password)
{
    CGuldenSecretExt<CExtKey> linkedKey;
    if (!linkedKey.fromURIString(linked_uri))
    {
        return false;
    }
    GuldenAppManager::gApp->setLinkKey(linkedKey);
    GuldenAppManager::gApp->isLink = true;
    GuldenAppManager::gApp->setRecoveryPassword(password.c_str());
    GuldenAppManager::gApp->initialize();

    return true;
}

bool GuldenUnifiedBackend::ContinueWalletLinkedFromURI(const std::string & linked_uri, const std::string& password)
{
    if (!pactiveWallet)
    {
        LogPrintf("%s: No active wallet", __func__);
        return false;
    }

    LOCK2(cs_main, pactiveWallet->cs_wallet);

    CGuldenSecretExt<CExtKey> linkedKey;
    if (!linkedKey.fromURIString(linked_uri))
    {
        LogPrintf("%s: Failed to parse link URI", __func__);
        return false;
    }

    GuldenAppManager::gApp->setLinkKey(linkedKey);
    GuldenAppManager::gApp->setRecoveryPassword(password.c_str());
    GuldenAppManager::gApp->isLink = true;

    CWallet::CreateSeedAndAccountFromLink(pactiveWallet);

    // Allow update of balance for deleted accounts/transactions
    LogPrintf("%s: Update balance and rescan", __func__);
    notifyBalanceChanged(pactiveWallet);

    // Rescan for transactions on the linked account
    DoRescan();

    return true;
}

bool GuldenUnifiedBackend::ReplaceWalletLinkedFromURI(const std::string& linked_uri, const std::string& password)
{
    LOCK2(cs_main, pactiveWallet->cs_wallet);

    if (!pactiveWallet || !pactiveWallet->activeAccount)
    {
        LogPrintf("ReplaceWalletLinkedFromURI: No active wallet");
        return false;
    }

    // Create ext key for new linked account from parsed data
    CGuldenSecretExt<CExtKey> linkedKey;
    if (!linkedKey.fromURIString(linked_uri))
    {
        LogPrintf("ReplaceWalletLinkedFromURI: Failed to parse link URI");
        return false;
    }

    // Ensure we have a valid location to send all the funds
    CGuldenAddress address(linkedKey.getPayAccount());
    if (!address.IsValid())
    {
        LogPrintf("ReplaceWalletLinkedFromURI: invalid address %s", linkedKey.getPayAccount().c_str());
        return false;
    }

    // Empty wallet to target address
    LogPrintf("ReplaceWalletLinkedFromURI: Empty accounts into linked address");
    bool fSubtractFeeFromAmount = true;
    std::vector<std::tuple<CWalletTx*, CReserveKeyOrScript*>> transactionsToCommit;
    for (const auto& [accountUUID, pAccount] : pactiveWallet->mapAccounts)
    {
        CAmount nBalance = pactiveWallet->GetBalance(pAccount, false, true, true);
        if (nBalance > 0)
        {
            LogPrintf("ReplaceWalletLinkedFromURI: Empty account into linked address [%s]", getUUIDAsString(accountUUID).c_str());
            std::vector<CRecipient> vecSend;
            CRecipient recipient = GetRecipientForDestination(address.Get(), nBalance, fSubtractFeeFromAmount, GetPoW2Phase(chainActive.Tip(), Params(), chainActive));
            vecSend.push_back(recipient);

            CWalletTx* pWallettx = new CWalletTx();
            CAmount nFeeRequired;
            int nChangePosRet = -1;
            std::string strError;
            CReserveKeyOrScript* pReserveKey = new CReserveKeyOrScript(pactiveWallet, pAccount, KEYCHAIN_CHANGE);
            if (!pactiveWallet->CreateTransaction(pAccount, vecSend, *pWallettx, *pReserveKey, nFeeRequired, nChangePosRet, strError))
            {
                LogPrintf("ReplaceWalletLinkedFromURI: Failed to create transaction %s [%d]",strError.c_str(), nBalance);
                return false;
            }
            transactionsToCommit.push_back(std::tuple(pWallettx, pReserveKey));
        }
        else
        {
            LogPrintf("ReplaceWalletLinkedFromURI: Account already empty [%s]", getUUIDAsString(accountUUID).c_str());
        }
    }

    if (!EraseWalletSeedsAndAccounts())
    {
        LogPrintf("ReplaceWalletLinkedFromURI: Failed to erase seed and accounts");
        return false;
    }

    GuldenAppManager::gApp->setLinkKey(linkedKey);
    GuldenAppManager::gApp->setRecoveryPassword(password.c_str());
    GuldenAppManager::gApp->isLink = true;

    CWallet::CreateSeedAndAccountFromLink(pactiveWallet);

    for (auto& [pWalletTx, pReserveKey] : transactionsToCommit)
    {
        CValidationState state;
        //NB! We delibritely pass nullptr for connman here to prevent transaction from relaying
        //We allow the relaying to occur inside DoRescan instead
        if (!pactiveWallet->CommitTransaction(*pWalletTx, *pReserveKey, nullptr, state))
        {
            LogPrintf("ReplaceWalletLinkedFromURI: Failed to commit transaction");
            return false;
        }
        delete pWalletTx;
        delete pReserveKey;
    }

    // Allow update of balance for deleted accounts/transactions
    LogPrintf("ReplaceWalletLinkedFromURI: Update balance and rescan");
    notifyBalanceChanged(pactiveWallet);

    // Rescan for transactions on the linked account
    DoRescan();

    return true;
}

bool GuldenUnifiedBackend::EraseWalletSeedsAndAccounts()
{
    pactiveWallet->EraseWalletSeedsAndAccounts();
    return true;
}

bool GuldenUnifiedBackend::IsValidLinkURI(const std::string& linked_uri)
{
    CGuldenSecretExt<CExtKey> linkedKey;
    if (!linkedKey.fromURIString(linked_uri))
        return false;
    return true;
}


int32_t GuldenUnifiedBackend::InitUnityLib(const std::string& dataDir, const std::string& staticFilterPath, int64_t staticFilterOffset, int64_t staticFilterLength, bool testnet, const std::shared_ptr<GuldenUnifiedFrontend>& signals, const std::string& extraArgs)
{
    // Force the datadir to specific place on e.g. android devices
    if (!dataDir.empty())
        SoftSetArg("-datadir", dataDir);

    // SPV wallets definitely shouldn't be listening for incoming connections at all
    SoftSetArg("-listen", "0");

    // Mininmise logging for performance reasons
    SoftSetArg("-debug", "0");

    // Turn SPV mode on
    SoftSetArg("-fullsync", "0");
    SoftSetArg("-spv", "1");
    SoftSetArg("-spvstaticfilterfile", staticFilterPath);
    SoftSetArg("-spvstaticfilterfileoffset", i64tostr(staticFilterOffset));
    SoftSetArg("-spvstaticfilterfilelength", i64tostr(staticFilterLength));

    // Minimise lookahead size for performance reasons
    SoftSetArg("-accountpool", "1");

    // Minimise background threads and memory consumption
    SoftSetArg("-par", "-100");
    SoftSetArg("-maxsigcachesize", "0");
    SoftSetArg("-dbcache", "4");
    SoftSetArg("-maxmempool", "5");
    SoftSetArg("-maxconnections", "8");

    //fixme: (2.1) (UNITY) (iOS) - Temporarily hardcoding for android, set this on a per app basis.
    // Change client name
    SoftSetArg("-clientname", "Gulden android");

    // Testnet
    if (testnet)
    {
        SoftSetArg("-testnet", "C1534687770:60");
        SoftSetArg("-addnode", "devbak.net");
    }

    //fixme: (2.1) Reverse headers
    // Temporarily disable reverse headers for mobile until memory requirements can be reduced.
    SoftSetArg("-reverseheaders", "false");

    signalHandler = signals;

    if (!extraArgs.empty()) {
        std::vector<const char*> args;
        auto splitted = boost::program_options::split_unix(extraArgs);
        for(const auto& part: splitted)
            args.push_back(part.c_str());
        gArgs.ParseExtraParameters(int(args.size()), args.data());
    }

    return InitUnity();
}

void GuldenUnifiedBackend::TerminateUnityLib()
{
    GuldenAppManager::gApp->shutdown();
    GuldenAppManager::gApp->waitForShutDown();
}

QrCodeRecord GuldenUnifiedBackend::QRImageFromString(const std::string& qr_string, int32_t width_hint)
{
    QRcode* code = QRcode_encodeString(qr_string.c_str(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
    if (!code)
    {
        return QrCodeRecord(0, std::vector<uint8_t>());
    }
    else
    {
        const int32_t generatedWidth = code->width;
        const int32_t finalWidth = (width_hint / generatedWidth) * generatedWidth;
        const int32_t scaleMultiplier = finalWidth / generatedWidth;
        std::vector<uint8_t> dataVector;
        dataVector.reserve(finalWidth*finalWidth);
        int nIndex=0;
        for (int nRow=0; nRow<generatedWidth; ++nRow)
        {
            for (int nCol=0; nCol<generatedWidth; ++nCol)
            {
                dataVector.insert(dataVector.end(), scaleMultiplier, (code->data[nIndex++] & 1) * 255);
            }
            for (int i=1; i<scaleMultiplier; ++i)
            {
                dataVector.insert(dataVector.end(), dataVector.end()-finalWidth, dataVector.end());
            }
        }
        QRcode_free(code);
        return QrCodeRecord(finalWidth, dataVector);
    }
}

std::string GuldenUnifiedBackend::GetReceiveAddress()
{
    LOCK2(cs_main, pactiveWallet->cs_wallet);

    if (!pactiveWallet || !pactiveWallet->activeAccount)
        return "";

    CReserveKeyOrScript* receiveAddress = new CReserveKeyOrScript(pactiveWallet, pactiveWallet->activeAccount, KEYCHAIN_EXTERNAL);
    CPubKey pubKey;
    if (receiveAddress->GetReservedKey(pubKey))
    {
        CKeyID keyID = pubKey.GetID();
        receiveAddress->ReturnKey();
        delete receiveAddress;
        return CGuldenAddress(keyID).ToString();
    }
    else
    {
        return "";
    }
}

//fixme: (Unity) - find a way to use char[] here as well as on the java side.
std::string GuldenUnifiedBackend::GetRecoveryPhrase()
{
    if (!pactiveWallet || !pactiveWallet->activeAccount)
        return "";

    LOCK2(cs_main, pactiveWallet->cs_wallet);
    //WalletModel::UnlockContext ctx(walletModel->requestUnlock());
    //if (ctx.isValid())
    {
        int64_t birthTime = pactiveWallet->birthTime();

        std::set<SecureString> allPhrases;
        for (const auto& seedIter : pactiveWallet->mapSeeds)
        {
            return GuldenAppManager::composeRecoveryPhrase(seedIter.second->getMnemonic(), birthTime).c_str();
        }
    }
    return "";
}

bool GuldenUnifiedBackend::IsMnemonicWallet()
{
    if (!pactiveWallet || !pactiveWallet->activeAccount)
        throw std::runtime_error(_("No active internal wallet."));

    LOCK2(cs_main, pactiveWallet->cs_wallet);

    return pactiveWallet->activeSeed != nullptr;
}

bool GuldenUnifiedBackend::IsMnemonicCorrect(const std::string & phrase)
{
    if (!pactiveWallet || !pactiveWallet->activeAccount)
        throw std::runtime_error(_("No active internal wallet."));

    SecureString mnemonicPhrase;
    int birthNumber;

    GuldenAppManager::splitRecoveryPhraseAndBirth(SecureString(phrase), mnemonicPhrase, birthNumber);

    LOCK2(cs_main, pactiveWallet->cs_wallet);

    for (const auto& seedIter : pactiveWallet->mapSeeds)
    {
        if (mnemonicPhrase == seedIter.second->getMnemonic())
            return true;
    }
    return false;
}


//fixme: (2.1) HIGH - take a timeout value and always lock again after timeout
bool GuldenUnifiedBackend::UnlockWallet(const std::string& password)
{
    if (!pactiveWallet)
    {
        LogPrintf("UnlockWallet: No active wallet");
        return false;
    }

    if (!dynamic_cast<CGuldenWallet*>(pactiveWallet)->IsCrypted())
    {
        LogPrintf("UnlockWallet: Wallet not encrypted");
        return false;
    }

    return pactiveWallet->Unlock(password.c_str());
}

bool GuldenUnifiedBackend::LockWallet()
{
    if (!pactiveWallet)
    {
        LogPrintf("LockWallet: No active wallet");
        return false;
    }

    if (dynamic_cast<CGuldenWallet*>(pactiveWallet)->IsLocked())
        return true;

    return dynamic_cast<CGuldenWallet*>(pactiveWallet)->Lock();
}

bool GuldenUnifiedBackend::ChangePassword(const std::string& oldPassword, const std::string& newPassword)
{
    if (!pactiveWallet)
    {
        LogPrintf("ChangePassword: No active wallet");
        return false;
    }

    if (newPassword.length() == 0)
    {
        LogPrintf("ChangePassword: Refusing invalid password of length 0");
        return false;
    }

    return pactiveWallet->ChangeWalletPassphrase(oldPassword.c_str(), newPassword.c_str());
}

bool GuldenUnifiedBackend::HaveUnconfirmedFunds()
{
    if (!pactiveWallet)
        return true;

    WalletBalances balances;
    pactiveWallet->GetBalances(balances, pactiveWallet->activeAccount, true);

    if (balances.unconfirmedIncludingLocked > 0 || balances.immatureIncludingLocked > 0)
    {
        return true;
    }
    return false;
}

int64_t GuldenUnifiedBackend::GetBalance()
{
    if (!pactiveWallet)
        return 0;

    WalletBalances balances;
    pactiveWallet->GetBalances(balances, pactiveWallet->activeAccount, true);
    return balances.availableIncludingLocked + balances.unconfirmedIncludingLocked + balances.immatureIncludingLocked;
}

void GuldenUnifiedBackend::DoRescan()
{
    if (pactiveWallet)
    {
        ResetSPVStartRescanThread();
    }
}

UriRecipient GuldenUnifiedBackend::IsValidRecipient(const UriRecord & request)
{
     // return if URI is not valid or is no Gulden: URI
    std::string lowerCaseScheme = boost::algorithm::to_lower_copy(request.scheme);
    if (lowerCaseScheme != "guldencoin" && lowerCaseScheme != "gulden")
        return UriRecipient(false, "", "", 0);

    if (!CGuldenAddress(request.path).IsValid())
        return UriRecipient(false, "", "", 0);

    std::string address = request.path;
    std::string label = "";
    CAmount amount = 0;
    if (request.items.find("amount") != request.items.end())
    {
        ParseMoney(request.items.find("amount")->second, amount);
    }

    if (pactiveWallet)
    {
        DS_LOCK2(cs_main, pactiveWallet->cs_wallet);
        if (pactiveWallet->mapAddressBook.find(address) != pactiveWallet->mapAddressBook.end())
        {
            label = pactiveWallet->mapAddressBook[address].name;
        }
    }

    return UriRecipient(true, address, label, amount);
}

PaymentResultStatus GuldenUnifiedBackend::performPaymentToRecipient(const UriRecipient & request, bool substract_fee)
{
    if (!pactiveWallet)
        throw std::runtime_error(_("No active internal wallet."));

    DS_LOCK2(cs_main, pactiveWallet->cs_wallet);

    CGuldenAddress address(request.address);
    if (!address.IsValid())
    {
        LogPrintf("performPaymentToRecipient: invalid address %s", request.address.c_str());
        throw std::runtime_error(_("Invalid address"));
    }

    CRecipient recipient = GetRecipientForDestination(address.Get(), request.amount, substract_fee, GetPoW2Phase(chainActive.Tip(), Params(), chainActive));
    std::vector<CRecipient> vecSend;
    vecSend.push_back(recipient);

    CWalletTx wtx;
    CAmount nFeeRequired;
    int nChangePosRet = -1;
    std::string strError;
    CReserveKeyOrScript reservekey(pactiveWallet, pactiveWallet->activeAccount, KEYCHAIN_CHANGE);
    if (!pactiveWallet->CreateTransaction(pactiveWallet->activeAccount, vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError))
    {
        if (!substract_fee && request.amount + nFeeRequired > GetBalance()) {
            return PaymentResultStatus::INSUFFICIENT_FUNDS;
        }
        LogPrintf("performPaymentToRecipient: failed to create transaction %s",strError.c_str());
        throw std::runtime_error(strprintf(_("Failed to create transaction\n%s"), strError));
    }

    CValidationState state;
    if (!pactiveWallet->CommitTransaction(wtx, reservekey, g_connman.get(), state))
    {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
        LogPrintf("performPaymentToRecipient: failed to commit transaction %s",strError.c_str());
        throw std::runtime_error(strprintf(_("Transaction rejected, reason: %s"), state.GetRejectReason()));
    }

    return PaymentResultStatus::SUCCESS;
}

std::vector<TransactionRecord> GuldenUnifiedBackend::getTransactionHistory()
{
    std::vector<TransactionRecord> ret;

    if (!pactiveWallet)
        return ret;

    DS_LOCK2(cs_main, pactiveWallet->cs_wallet);

    for (const auto& [hash, wtx] : pactiveWallet->mapWallet)
    {
        TransactionRecord tx = calculateTransactionRecordForWalletTransaction(wtx);
        ret.push_back(tx);
    }
    std::sort(ret.begin(), ret.end(), [&](TransactionRecord& x, TransactionRecord& y){ return (x.timeStamp > y.timeStamp); });
    return ret;
}

TransactionRecord GuldenUnifiedBackend::getTransaction(const std::string & txHash)
{
    if (!pactiveWallet)
        throw std::runtime_error(strprintf("No active wallet to query tx hash [%s]", txHash));

    uint256 hash = uint256S(txHash);

    DS_LOCK2(cs_main, pactiveWallet->cs_wallet);

    if (pactiveWallet->mapWallet.find(hash) == pactiveWallet->mapWallet.end())
        throw std::runtime_error(strprintf("No transaction found for hash [%s]", txHash));

    const CWalletTx& wtx = pactiveWallet->mapWallet[hash];
    return calculateTransactionRecordForWalletTransaction(wtx);
}

std::vector<MutationRecord> GuldenUnifiedBackend::getMutationHistory()
{
    std::vector<MutationRecord> ret;

    if (!pactiveWallet)
        return ret;

    DS_LOCK2(cs_main, pactiveWallet->cs_wallet);

    // wallet transactions in reverse chronological ordering
    std::vector<const CWalletTx*> vWtx;
    for (const auto& [hash, wtx] : pactiveWallet->mapWallet)
    {
        vWtx.push_back(&wtx);
    }
    std::sort(vWtx.begin(), vWtx.end(), [&](const CWalletTx* x, const CWalletTx* y){ return (x->nTimeSmart > y->nTimeSmart); });

    // build mutation list based on transactions
    for (const CWalletTx* wtx : vWtx)
    {
        addMutationsForTransaction(wtx, ret);
    }

    return ret;
}

std::vector<AddressRecord> GuldenUnifiedBackend::getAddressBookRecords()
{
    std::vector<AddressRecord> ret;
    if (pactiveWallet)
    {
        DS_LOCK2(cs_main, pactiveWallet->cs_wallet);
        for(const auto& [address, addressData] : pactiveWallet->mapAddressBook)
        {
            ret.emplace_back(AddressRecord(address, addressData.purpose, addressData.name));
        }
    }

    return ret;
}

void GuldenUnifiedBackend::addAddressBookRecord(const AddressRecord& address)
{
    if (pactiveWallet)
    {
        pactiveWallet->SetAddressBook(address.address, address.name, address.purpose);
    }
}

void GuldenUnifiedBackend::deleteAddressBookRecord(const AddressRecord& address)
{
    if (pactiveWallet)
    {
        pactiveWallet->DelAddressBook(address.address);
    }
}

void GuldenUnifiedBackend::PersistAndPruneForSPV()
{
    PersistAndPruneForPartialSync();
}

void GuldenUnifiedBackend::ResetUnifiedProgress()
{
    CWallet::ResetUnifiedSPVProgressNotification();
}

std::vector<PeerRecord> GuldenUnifiedBackend::getPeers()
{
    std::vector<PeerRecord> ret;

    if (g_connman) {
        std::vector<CNodeStats> vstats;
        g_connman->GetNodeStats(vstats);
        for (CNodeStats& nstat: vstats)
        {
            int64_t nSyncedHeight = 0;
            int64_t nCommonHeight = 0;
            CNodeStateStats stateStats;
            TRY_LOCK(cs_main, lockMain);
            if (lockMain && GetNodeStateStats(nstat.nodeid, stateStats))
            {
                nSyncedHeight = stateStats.nSyncHeight;
                nCommonHeight = stateStats.nCommonHeight;
            }

            PeerRecord rec(nstat.nodeid,
                           nstat.addr.ToString(),
                           nstat.addr.HostnameLookup(),
                           nstat.nStartingHeight,
                           nSyncedHeight,
                           nCommonHeight,
                           int32_t(nstat.dPingTime * 1000),
                           nstat.cleanSubVer,
                           nstat.nVersion);
            ret.emplace_back(rec);
        }
    }

    return ret;
}

std::vector<BlockInfoRecord> GuldenUnifiedBackend::getLastSPVBlockInfos()
{
    std::vector<BlockInfoRecord> ret;

    LOCK(cs_main);

    int height = partialChain.Height();
    while (ret.size() < 32 && height > partialChain.HeightOffset()) {
        const CBlockIndex* pindex = partialChain[height];
        ret.push_back(BlockInfoRecord(pindex->nHeight, pindex->GetBlockTime(), pindex->GetBlockHashPoW2().ToString()));
        height--;
    }

    return ret;
}

MonitorRecord GuldenUnifiedBackend::getMonitoringStats()
{
    LOCK(cs_main);
    int32_t partialHeight_ = partialChain.Height();
    int32_t partialOffset_ = partialChain.HeightOffset();
    int32_t prunedHeight_ = nPartialPruneHeightDone;
    int32_t processedSPVHeight_ = CSPVScanner::getProcessedHeight();
    int32_t probableHeight_ = GetProbableHeight();

    return MonitorRecord(partialHeight_,
                         partialOffset_,
                         prunedHeight_,
                         processedSPVHeight_,
                         probableHeight_);
}

void GuldenUnifiedBackend::RegisterMonitorListener(const std::shared_ptr<GuldenMonitorListener> & listener)
{
    LOCK(cs_monitoringListeners);
    monitoringListeners.insert(listener);
}

void GuldenUnifiedBackend::UnregisterMonitorListener(const std::shared_ptr<GuldenMonitorListener> & listener)
{
    LOCK(cs_monitoringListeners);
    monitoringListeners.erase(listener);
}
