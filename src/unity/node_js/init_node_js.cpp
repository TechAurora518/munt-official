// Copyright (c) 2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/thread.hpp>
#include "chain.h"
#include "init.h"

#include "unity/djinni/cpp/legacy_wallet_result.hpp"
#include "unity/djinni/cpp/gulden_unified_backend.hpp"

extern std::string HelpMessage(HelpMessageMode mode)
{
    return "";
}

void InitRegisterRPC()
{
}

void ServerInterrupt(boost::thread_group& threadGroup)
{
}

bool InitRPCWarmup(boost::thread_group& threadGroup)
{
    return true;
}

void SetRPCWarmupFinished()
{
}

void RPCNotifyBlockChange(bool ibd, const CBlockIndex * pindex)
{
}

void ServerShutdown(boost::thread_group& threadGroup)
{
}

void InitRPCMining()
{
}

bool InitTor(boost::thread_group& threadGroup, CScheduler& scheduler)
{
    return true;
}

bool GuldenUnifiedBackend::InitWalletFromAndroidLegacyProtoWallet(const std::string& walletFile, const std::string& oldPassword, const std::string& newPassword)
{
    // only exists here to keep the compiler happy, never call this on iOS
    LogPrintf("DO NOT call GuldenUnifiedBackend::InitWalletFromAndroidLegacyProtoWallet on iOS\n");
    assert(false);
}

LegacyWalletResult GuldenUnifiedBackend::isValidAndroidLegacyProtoWallet(const std::string& walletFile, const std::string& oldPassword)
{
    // only exists here to keep the compiler happy, never call this on iOS
    LogPrintf("DO NOT call GuldenUnifiedBackend::isValidAndroidLegacyProtoWallet on iOS\n");
    assert(false);
    return LegacyWalletResult::INVALID_OR_CORRUPT;
}
