// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <string>

struct BalanceRecord;
struct MutationRecord;
struct TransactionRecord;

/** Interface to receive events from the core */
class GuldenUnifiedFrontend {
public:
    virtual ~GuldenUnifiedFrontend() {}

    /**
     * Fraction of work done since session start or last progress reset [0..1]
     * Unified progress combines connection state, header and block sync
     */
    virtual void notifyUnifiedProgress(float progress) = 0;

    virtual void notifyBalanceChange(const BalanceRecord & new_balance) = 0;

    /**
     * Notification of new mutations.
     * If self_committed it is due to a call to performPaymentToRecipient, else it is because of a transaction
     * reached us in another way. In general this will be because we received funds from someone, hower there are
     * also cases where funds is send from our wallet while !self_committed (for example by a linked desktop wallet
     * or another wallet instance using the same keys as ours).
     */
    virtual void notifyNewMutation(const MutationRecord & mutation, bool self_committed) = 0;

    virtual void notifyUpdatedTransaction(const TransactionRecord & transaction) = 0;

    virtual void notifyInitWithExistingWallet() = 0;

    virtual void notifyInitWithoutExistingWallet() = 0;

    virtual void notifyShutdown() = 0;

    virtual void notifyCoreReady() = 0;

    virtual void logPrint(const std::string & str) = 0;
};
