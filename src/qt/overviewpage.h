// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GULDEN_QT_OVERVIEWPAGE_H
#define GULDEN_QT_OVERVIEWPAGE_H

#include "amount.h"

#include <QWidget>
#include <memory>

class ClientModel;
class TransactionFilterProxy;
class TxViewDelegate;
class QStyle;
class WalletModel;

namespace Ui {
    class OverviewPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(const QStyle *platformStyle, QWidget *parent = 0);
    ~OverviewPage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void showOutOfSyncWarning(bool fShow);

public Q_SLOTS:
    void setBalance(const CAmount& availableBalance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance, const CAmount& lockedBalance);

Q_SIGNALS:
    void transactionClicked(const QModelIndex &index);
    void outOfSyncWarningClicked();

private:
    Ui::OverviewPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CAmount currentAvailableBalance = -1;
    CAmount currentUnconfirmedBalance = -1;
    CAmount currentImmatureBalance = -1;
    CAmount currentWatchOnlyBalance = -1;
    CAmount currentWatchUnconfBalance = -1;
    CAmount currentWatchImmatureBalance = -1;
    CAmount currentLockedBalance = -1;

    TxViewDelegate *txdelegate;
    std::unique_ptr<TransactionFilterProxy> filter;

private Q_SLOTS:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
    void updateAlerts(const QString &warnings);
    void updateWatchOnlyLabels(bool showWatchOnly);
    void handleOutOfSyncWarningClicks();
};

#endif // GULDEN_QT_OVERVIEWPAGE_H
