// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_WALLETVIEW_H
#define GULDEN_QT_WALLETVIEW_H

#include "amount.h"

#include <QStackedWidget>

class GUI;
class ClientModel;
class OverviewPage;
class QStyle;
class ViewAddressDialog;
class ReceiveCoinsDialog;
class SendCoinsDialog;
class WitnessDialog;
class SendCoinsRecipient;
class TransactionView;
class WalletModel;
class AddressBookPage;

QT_BEGIN_NAMESPACE
class QModelIndex;
class QProgressDialog;
QT_END_NAMESPACE

/*
  WalletView class. This class represents the view to a single wallet.
  It was added to support multiple wallet functionality. Each wallet gets its own WalletView instance.
  It communicates with both the client and the wallet models to give the user an up-to-date view of the
  current core state.
*/
class WalletView : public QStackedWidget
{
    Q_OBJECT

public:
    explicit WalletView(const QStyle* platformStyle, QWidget* parent);
    virtual ~WalletView();

    void setGUI(GUI* gui);
    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel* clientModel);
    /** Set the wallet model.
        The wallet model represents a Gulden wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    void setWalletModel(WalletModel* walletModel);

    bool handlePaymentRequest(const SendCoinsRecipient& recipient);

    void showOutOfSyncWarning(bool fShow);

private:
    ClientModel* clientModel = nullptr;
    WalletModel* walletModel = nullptr;

    OverviewPage* overviewPage = nullptr;
    QWidget* transactionsPage = nullptr;
    ViewAddressDialog* viewAddressPage = nullptr;
    ReceiveCoinsDialog* receiveCoinsPage = nullptr;
    SendCoinsDialog* sendCoinsPage = nullptr;
    WitnessDialog* witnessDialogPage = nullptr;
    AddressBookPage* usedSendingAddressesPage = nullptr;
    AddressBookPage* usedReceivingAddressesPage = nullptr;

    TransactionView* transactionView = nullptr;

    QProgressDialog* progressDialog = nullptr;
    const QStyle* platformStyle = nullptr;

    friend class GuldenGUI;
    friend class GUI;

public Q_SLOTS:
    /** Switch to overview (home) page */
    void gotoOverviewPage();
    /** Switch to history (transactions) page */
    void gotoHistoryPage();
    /** Switch to view address page */
    void gotoViewAddressPage();
    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();
    /** Switch to witness information page */
    void gotoWitnessPage();
    /** Switch to send coins page */
    void gotoSendCoinsPage(QString addr = "");

    /** Show incoming transaction notification for new transactions.

        The new items are those between start and end inclusive, under the given parent item.
    */
    void processNewTransaction(const QModelIndex& parent, int start, int /*end*/);
    /** Encrypt the wallet */
    void encryptWallet(bool status);
    /** Backup the wallet */
    void backupWallet();
    /** Change encrypted wallet passphrase */
    void changePassphrase();
    /** Ask for passphrase to unlock wallet temporarily */
    void unlockWallet();

    /** Show used sending addresses */
    void usedSendingAddresses();
    /** Show used receiving addresses */
    void usedReceivingAddresses();

    /** Re-emit encryption status signal */
    void updateEncryptionStatus();

    /** Show progress dialog e.g. for rescan */
    void showProgress(const QString& title, int nProgress);

    /** User has requested more information about the out of sync state */
    void requestedSyncWarningInfo();

Q_SIGNALS:
    /** Signal that we want to show the main window */
    void showNormalIfMinimized();
    /**  Fired when a message should be reported to the user */
    void message(const QString& title, const QString& message, unsigned int style);
    /** Encryption status of wallet changed */
    void encryptionStatusChanged(int status);
    /** HD-Enabled status of wallet changed (only possible during startup) */
    /*void hdEnabledStatusChanged(int hdEnabled);*/
    /** Notify that a new transaction appeared */
    void incomingTransaction(const QString& date, int unit, const CAmount& amountReceived, const CAmount& amountSent, const QString& type, const QString& address, const QString& account, const QString& label);
    /** Notify that the out of sync warning icon has been pressed */
    void outOfSyncWarningClicked();
};

#endif // GULDEN_QT_WALLETVIEW_H
