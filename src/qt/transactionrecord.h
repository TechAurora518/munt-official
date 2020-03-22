// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_TRANSACTIONRECORD_H
#define GULDEN_QT_TRANSACTIONRECORD_H

#include "amount.h"
#include "uint256.h"

#include <QList>
#include <QString>
#define BOOST_UUID_RANDOM_PROVIDER_FORCE_POSIX
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>

class CWallet;
class CWalletTx;

/** UI model for transaction status. The transaction status is the part of a transaction that will change over time.
 */
class TransactionStatus
{
public:
    TransactionStatus():
        countsForBalance(false), sortKey(""),
        matures_in(0), status(Unconfirmed), depth(0), open_for(0), cur_num_blocks(-1)
    { }

    enum Status {
        Confirmed,          /**< Have 6 or more confirmations (normal tx) or fully mature (mined tx) **/
        /// Normal (sent/received) transactions
        OpenUntilDate,      /**< Transaction not yet final, waiting for date */
        OpenUntilBlock,     /**< Transaction not yet final, waiting for block */
        Unconfirmed,        /**< Not yet mined into a block **/
        Confirming,         /**< Confirmed, but waiting for the recommended number of confirmations **/
        Conflicted,         /**< Conflicts with other transaction or mempool **/
        Abandoned,          /**< Abandoned from the wallet **/
        /// Generated (mined) transactions
        Immature,           /**< Mined but waiting for maturity */
        NotAccepted         /**< Mined but not accepted */
    };

    /// Transaction counts towards available balance
    bool countsForBalance;
    /// Sorting key based on status
    std::string sortKey;

    /** @name Generated (mined) transactions
       @{*/
    int matures_in;
    /**@}*/

    /** @name Reported status
       @{*/
    Status status;
    qint64 depth;
    qint64 open_for; /**< Timestamp if status==OpenUntilDate, otherwise number
                      of additional blocks that need to be mined before
                      finalization */
    /**@}*/

    /** Current number of blocks (to know whether cached status is still valid) */
    int cur_num_blocks;

    bool needsUpdate;
};

/** UI model for a transaction. A core transaction can be represented by multiple UI transactions if it has
    multiple outputs.
 */
class TransactionRecord
{
public:
    enum Type
    {
        Other,
        Generated,
        GeneratedWitness,
        WitnessRenew,
        WitnessFundSend,
        WitnessFundRecv,
        WitnessEmptySend,
        WitnessEmptyRecv,
        WitnessRearrangeRecv,
        WitnessIncreaseRecv,
        WitnessIncreaseSend,
        WitnessChangeKeyRecv,
        SendToAddress,
        SendToOther,
        RecvWithAddress,
        RecvFromOther,
        SendToSelf,
        InternalTransfer
    };

    /** Number of confirmation recommended for accepting a transaction */
    static const int RecommendedNumConfirmations = 3;

    TransactionRecord():
            hash(), time(0), type(Other), address(""), debit(0), credit(0), fee(0), idx(0), actionAccountUUID(boost::uuids::nil_generator()()), actionAccountParentUUID(boost::uuids::nil_generator()()), fromAccountUUID(boost::uuids::nil_generator()()), fromAccountParentUUID(boost::uuids::nil_generator()()), receiveAccountUUID(boost::uuids::nil_generator()()), receiveAccountParentUUID(boost::uuids::nil_generator()())
    {
    }

    TransactionRecord(uint256 _hash, qint64 _time):
            hash(_hash), time(_time), type(Other), address(""), debit(0),
            credit(0), fee(0), idx(0), actionAccountUUID(boost::uuids::nil_generator()()), actionAccountParentUUID(boost::uuids::nil_generator()()), fromAccountUUID(boost::uuids::nil_generator()()), fromAccountParentUUID(boost::uuids::nil_generator()()), receiveAccountUUID(boost::uuids::nil_generator()()), receiveAccountParentUUID(boost::uuids::nil_generator()())
    {
    }

    TransactionRecord(uint256 _hash, qint64 _time,
                Type _type, const std::string &_address,
                const CAmount& _debit, const CAmount& _credit):
            hash(_hash), time(_time), type(_type), address(_address), debit(_debit), credit(_credit),
            idx(0), actionAccountUUID(boost::uuids::nil_generator()()), actionAccountParentUUID(boost::uuids::nil_generator()()), fromAccountUUID(boost::uuids::nil_generator()()), fromAccountParentUUID(boost::uuids::nil_generator()()), receiveAccountUUID(boost::uuids::nil_generator()()), receiveAccountParentUUID(boost::uuids::nil_generator()())
    {
    }

    /** Decompose CWallet transaction to model transaction records.
     */
    static bool showTransaction(const CWalletTx &wtx);
    static QList<TransactionRecord> decomposeTransaction(const CWallet *wallet, const CWalletTx &wtx);

    /** @name Immutable transaction attributes
      @{*/
    uint256 hash;
    qint64 time;
    Type type;
    std::string address;
    CAmount debit;
    CAmount credit;
    CAmount fee;
    /**@}*/

    /** Subtransaction index, for sort key */
    int idx;

    //Which account the record belongs too - i.e. a send belongs to a sender account while a receive belongs to a receiver account.
    boost::uuids::uuid actionAccountUUID;
    boost::uuids::uuid actionAccountParentUUID;
    //Sender account
    boost::uuids::uuid fromAccountUUID;
    boost::uuids::uuid fromAccountParentUUID;
    //Receiver account
    boost::uuids::uuid receiveAccountUUID;
    boost::uuids::uuid receiveAccountParentUUID;

    /** Status: can change with block chain update */
    TransactionStatus status;

    /** Whether the transaction was sent/received with a watch-only address */
    bool involvesWatchAddress;

    /** Return the unique identifier for this transaction (part) */
    QString getTxID() const;

    /** Return the output index of the subtransaction  */
    int getOutputIndex() const;

    /** Update status from core wallet tx.
     */
    void updateStatus(const CWalletTx &wtx);

    /** Return whether a status update is needed.
     */
    bool statusUpdateNeeded();
};

#endif // GULDEN_QT_TRANSACTIONRECORD_H
