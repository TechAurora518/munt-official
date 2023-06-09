// Copyright (c) 2018-2022 The Centure developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "appname.h"
#include <boost/thread.hpp>
#include "chain.h"
#include "init.h"
#include "node/context.h"

#include "unity/compat/android_wallet.h"
#include "unity/djinni/cpp/legacy_wallet_result.hpp"
#include "unity/djinni/cpp/i_library_controller.hpp"
#include "mnemonic_record.hpp"

extern std::string HelpMessage(HelpMessageMode mode)
{
    return "";
}

void InitRegisterRPC()
{
}

void ServerInterrupt()
{
}

bool InitRPCWarmup()
{
    return true;
}

void SetRPCWarmupFinished()
{
}

void RPCNotifyBlockChange(bool ibd, const CBlockIndex * pindex)
{
}

void ServerShutdown(node::NodeContext& nodeContext)
{
    // After everything has been shut down, but before things get flushed, stop the
    // CScheduler/checkqueue, scheduler and load block thread.
    if (nodeContext.scheduler) nodeContext.scheduler->stop();
}

void InitRPCMining()
{
}

bool InitTor()
{
    return true;
}


bool ILibraryController::InitWalletFromAndroidLegacyProtoWallet(const std::string& walletFile, const std::string& oldPassword, const std::string& newPassword)
{
    android_wallet wallet = ParseAndroidProtoWallet(walletFile, oldPassword);
    if (wallet.validWalletProto && wallet.validWallet)
    {
        if (wallet.walletSeedMnemonic.length() > 0)
        {
            if (wallet.walletSeedMnemonic.find("-") != std::string::npos && wallet.walletSeedMnemonic.find(":") != std::string::npos)
            {
                return InitWalletLinkedFromURI(GLOBAL_APP_URIPREFIX"sync:"+wallet.walletSeedMnemonic+";unused_payout_address", newPassword.c_str());
            }
            else
            {
                if (wallet.walletBirth > 0)
                {
                    return InitWalletFromRecoveryPhrase(ComposeRecoveryPhrase(wallet.walletSeedMnemonic, wallet.walletBirth).phrase_with_birth_number.c_str(), newPassword.c_str());
                }
                else
                {
                    return InitWalletFromRecoveryPhrase(wallet.walletSeedMnemonic.c_str(), newPassword.c_str());
                }
            }
        }
    }
    return false;
}

LegacyWalletResult ILibraryController::isValidAndroidLegacyProtoWallet(const std::string& walletFile, const std::string& oldPassword)
{
    LogPrintf("Checking for valid legacy wallet proto [%s]\n", walletFile.c_str());

    android_wallet wallet = ParseAndroidProtoWallet(walletFile, oldPassword);

    if (wallet.validWalletProto)
    {
        LogPrintf("Valid proto found\n");

        if (wallet.encrypted)
        {
            LogPrintf("Proto is encrypted\n");

            if (!wallet.validWallet)
            {
                if ( oldPassword.length() == 0 )
                {
                    LogPrintf("Password required\n");
                    return LegacyWalletResult::ENCRYPTED_PASSWORD_REQUIRED;
                }
                LogPrintf("Password is invalid\n");
                return LegacyWalletResult::PASSWORD_INVALID;
            }
        }
        if (wallet.walletSeedMnemonic.length() > 0)
        {
            LogPrintf("Wallet is valid\n");
            return LegacyWalletResult::VALID;
        }
    }
    LogPrintf("Wallet invalid or corrupt [%s]\n", wallet.resultMessage);
    return LegacyWalletResult::INVALID_OR_CORRUPT;
}
