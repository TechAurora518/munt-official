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

#include "walletview.h"

#include "addressbookpage.h"
#include "askpassphrasedialog.h"
#include "gui.h"
#include "clientmodel.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "overviewpage.h"
#include "qt/viewaddressdialog.h"
#include "qt/receivecoinsdialog.h"
#include "qt/witnessdialog.h"
#include "qt/miningaccountdialog.h"
#include "qt/GuldenGUI.h"
#include "sendcoinsdialog.h"
#include "transactiontablemodel.h"
#include "transactionview.h"
#include "walletmodel.h"



#include "ui_interface.h"

#include <QAction>
#include <QActionGroup>
#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QToolButton>

#define BOOST_UUID_RANDOM_PROVIDER_FORCE_POSIX
#include <boost/uuid/nil_generator.hpp>



WalletView::WalletView(const QStyle *_platformStyle, QWidget *parent)
: QStackedWidget(parent)
, clientModel(0)
, walletModel(0)
, platformStyle(_platformStyle)
{
    // Create tabs
    overviewPage = new OverviewPage(platformStyle);

    setContentsMargins( 0, 0, 0, 0);

    transactionsPage = new QWidget(this);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    transactionView = new TransactionView(this);
    transactionsPage->setObjectName("transactionView");
    transactionView->setObjectName("transactionViewTable");
    vbox->addWidget(transactionView,6);
     
    QFrame* horizontalButtonFrame = new QFrame(this);
    horizontalButtonFrame->setContentsMargins(0, 0, 0, 0);
    horizontalButtonFrame->setObjectName("transactionHistoryButtonFrame");
    QHBoxLayout* hbox_buttons = new QHBoxLayout();
    hbox_buttons->setSpacing(0);
    hbox_buttons->setContentsMargins(0, 0, 0, 0);
    
    orphanCheckbox = new QCheckBox(tr("Show orphaned rewards"), this);
    orphanCheckbox->setToolTip(tr("Control display of orphaned rewards.\n\nOrphaned rewards naturally occur from time to time.\nGenerally when someone else generates a block at the same time as yours."));
    orphanCheckbox->setObjectName("transactionShowOrphansCheckbox");
    orphanCheckbox->setCursor( Qt::PointingHandCursor );
    orphanCheckbox->setContentsMargins(0, 0, 0, 0);
    hbox_buttons->addWidget(orphanCheckbox);
    
    hbox_buttons->addStretch();

    QPushButton* exportButton = new QPushButton(tr("&Export"), this);
    exportButton->setObjectName("transactionExportButton");
    exportButton->setCursor( Qt::PointingHandCursor );
    exportButton->setToolTip(tr("Export the current list of transactions to a file."));
    hbox_buttons->addWidget(exportButton);
    horizontalButtonFrame->setLayout(hbox_buttons);
    vbox->addWidget(horizontalButtonFrame);
    

    transactionsPage->setLayout(vbox);
    transactionsPage->setContentsMargins(0, 0, 0, 0);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);


    viewAddressPage = new ViewAddressDialog(platformStyle);
    receiveCoinsPage = new ReceiveCoinsDialog(platformStyle);
    sendCoinsPage = new SendCoinsDialog(platformStyle);
    witnessDialogPage = new WitnessDialog(platformStyle);
    miningDialogPage = new MiningAccountDialog(platformStyle);

    usedSendingAddressesPage = new AddressBookPage(platformStyle, AddressBookPage::ForEditing, AddressBookPage::SendingTab, this);
    usedReceivingAddressesPage = new AddressBookPage(platformStyle, AddressBookPage::ForEditing, AddressBookPage::ReceivingTab, this);

    addWidget(overviewPage);
    addWidget(transactionsPage);
    addWidget(viewAddressPage);
    addWidget(receiveCoinsPage);
    addWidget(sendCoinsPage);
    addWidget(witnessDialogPage);
    addWidget(miningDialogPage);

    // Clicking on a transaction on the overview pre-selects the transaction on the transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));
    connect(overviewPage, SIGNAL(outOfSyncWarningClicked()), this, SLOT(requestedSyncWarningInfo()));

    // Double-clicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    // Clicking on "Export" allows to export the transaction list
    connect(exportButton, SIGNAL(clicked()), transactionView, SLOT(exportClicked()));

    // Pass through messages from sendCoinsPage
    connect(sendCoinsPage, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));

    // Pass through messages from witnessDialogPage
    connect(witnessDialogPage, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));
    
    // Pass through messages from miningDialogPage
    connect(miningDialogPage, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));
    
    // Pass through messages from transactionView
    connect(transactionView, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));
    
    // Toggle orphan visibility in transaction view
    connect(orphanCheckbox, SIGNAL(clicked(bool)), transactionView, SLOT(toggleShowOrphans(bool)));
}

WalletView::~WalletView()
{
    LogPrintf("WalletView::~WalletView\n");
}

void WalletView::setGUI(GUI *gui)
{
    if (gui)
    {
        // Clicking on a transaction on the overview page simply sends you to transaction history page
        connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), gui, SLOT(gotoHistoryPage()));

        // Receive and report messages
        connect(this, SIGNAL(message(QString,QString,unsigned int)), gui, SLOT(message(QString,QString,unsigned int)));

        // Pass through encryption status changed signals
        connect(this, SIGNAL(encryptionStatusChanged(int)), gui, SLOT(setEncryptionStatus(int)));

        // Pass through transaction notifications
        connect(this, SIGNAL(incomingTransaction(QString,int,CAmount,CAmount,QString,QString,QString,QString)), gui, SLOT(incomingTransaction(QString,int,CAmount,CAmount,QString,QString,QString,QString)));
    }
}

void WalletView::setClientModel(ClientModel *_clientModel)
{
    this->clientModel = _clientModel;

    overviewPage->setClientModel(_clientModel);
    sendCoinsPage->setClientModel(_clientModel);
    witnessDialogPage->setClientModel(_clientModel);
    miningDialogPage->setClientModel(_clientModel);
}

void WalletView::setWalletModel(WalletModel *_walletModel)
{
    this->walletModel = _walletModel;

    // Put transaction list in tabs
    transactionView->setModel(_walletModel);
    overviewPage->setWalletModel(_walletModel);
    receiveCoinsPage->setModel(_walletModel);
    sendCoinsPage->setModel(_walletModel);
    witnessDialogPage->setModel(_walletModel);
    miningDialogPage->setModel(_walletModel);
    usedReceivingAddressesPage->setModel(_walletModel->getAddressTableModel());
    usedSendingAddressesPage->setModel(_walletModel->getAddressTableModel());

    if (_walletModel)
    {
        // Handle active account changes.
        connect(_walletModel, SIGNAL(activeAccountChanged(CAccount*)), this, SLOT(activeAccountChanged(CAccount*)));
        activeAccountChanged(_walletModel->getActiveAccount());
        
        // Receive and pass through messages from wallet model
        connect(_walletModel, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));

        // Handle changes in encryption status
        connect(_walletModel, SIGNAL(encryptionStatusChanged(int)), this, SIGNAL(encryptionStatusChanged(int)));
        updateEncryptionStatus();

        // Balloon pop-up for new transaction
        connect(_walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(processNewTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(_walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));

        if (chainActive.Tip() && chainActive.TipPrev())
        {
            if (witnessDialogPage)
                witnessDialogPage->updateAccountIndicators();
        }
    }
}


void WalletView::processNewTransaction(const QModelIndex& parent, int start, int end)
{
    // Prevent balloon-spam when initial block download is in progress
    if (!walletModel || !clientModel || clientModel->inInitialBlockDownload())
        return;

    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    if (!ttm || ttm->processingQueuedTransactions())
        return;

    for(int idx = start; idx<=end; ++idx )
    {
        QString date = ttm->index(idx, TransactionTableModel::Date, parent).data().toString();
        qint64 amountReceived = ttm->index(idx, TransactionTableModel::AmountReceived, parent).data(Qt::EditRole).toULongLong();
        qint64 amountSent = ttm->index(idx, TransactionTableModel::AmountSent, parent).data(Qt::EditRole).toULongLong();
        QString type = ttm->index(idx, TransactionTableModel::Type, parent).data().toString();
        QModelIndex index = ttm->index(idx, 0, parent);
        QString address = ttm->data(index, TransactionTableModel::AddressRole).toString();
        QString label = ttm->data(index, TransactionTableModel::LabelRole).toString();


        boost::uuids::uuid accountUUID = ttm->data(index, TransactionTableModel::AccountRole).value<boost::uuids::uuid>();
        if (fShowChildAccountsSeperately)
        {
            boost::uuids::uuid accountParentUUID = ttm->data(index, TransactionTableModel::AccountParentRole).value<boost::uuids::uuid>();
            if (accountParentUUID != boost::uuids::nil_generator()())
                accountUUID = accountParentUUID;
        }

        QString accountLabel;
        if(accountUUID != boost::uuids::nil_generator()())
        {
            accountLabel = walletModel->getAccountLabel(accountUUID);
        }

        Q_EMIT incomingTransaction(date, walletModel->getOptionsModel()->getDisplayUnit(), amountReceived, amountSent, type, address, accountLabel, label);
    }
}

void WalletView::activeAccountChanged(CAccount* account)
{
    if (!account || !(account->IsPoW2Witness() || account->IsMiningAccount()))
    {
        orphanCheckbox->setVisible(false);
    }
    else
    {
        orphanCheckbox->setVisible(true);
    }
}

void WalletView::gotoOverviewPage()
{
    setCurrentWidget(overviewPage);
}

void WalletView::gotoHistoryPage()
{
    setCurrentWidget(transactionsPage);
}


void WalletView::gotoViewAddressPage()
{
    setCurrentWidget(viewAddressPage);
}

void WalletView::gotoReceiveCoinsPage()
{
    setCurrentWidget(receiveCoinsPage);
    receiveCoinsPage->gotoReceievePage();
}

void WalletView::gotoWitnessPage()
{
    setCurrentWidget(witnessDialogPage);
}

void WalletView::gotoMiningPage()
{
    setCurrentWidget(miningDialogPage);
}

void WalletView::gotoSendCoinsPage(QString addr)
{
    setCurrentWidget(sendCoinsPage);

    if (!addr.isEmpty())
        sendCoinsPage->setAddress(addr);
}

bool WalletView::handlePaymentRequest(const SendCoinsRecipient& recipient)
{
    return sendCoinsPage->handlePaymentRequest(recipient);
}

void WalletView::showOutOfSyncWarning(bool fShow)
{
    overviewPage->showOutOfSyncWarning(fShow);
}

void WalletView::updateEncryptionStatus()
{
    Q_EMIT encryptionStatusChanged(walletModel->getEncryptionStatus());
}

void WalletView::encryptWallet(bool status)
{
    if(!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt : AskPassphraseDialog::Decrypt, this);
    dlg.setModel(walletModel);
    dlg.exec();

    updateEncryptionStatus();
}

void WalletView::backupWallet()
{
    QString filename = GUIUtil::getSaveFileName(this,
        tr("Backup Wallet"), QString(),
        tr("Wallet Data (*.dat)"), NULL);

    if (filename.isEmpty())
        return;

    if (!walletModel->backupWallet(filename)) {
        Q_EMIT message(tr("Backup Failed"), tr("There was an error trying to save the wallet data to %1.").arg(filename),
            CClientUIInterface::MSG_ERROR);
        }
    else {
        Q_EMIT message(tr("Backup Successful"), tr("The wallet data was successfully saved to %1.").arg(filename),
            CClientUIInterface::MSG_INFORMATION);
    }
}

void WalletView::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void WalletView::unlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if (walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog dlg(AskPassphraseDialog::Unlock, this);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void WalletView::usedSendingAddresses()
{
    if(!walletModel)
        return;

    usedSendingAddressesPage->show();
    usedSendingAddressesPage->raise();
    usedSendingAddressesPage->activateWindow();
}

void WalletView::usedReceivingAddresses()
{
    if(!walletModel)
        return;

    usedReceivingAddressesPage->show();
    usedReceivingAddressesPage->raise();
    usedReceivingAddressesPage->activateWindow();
}

void WalletView::showProgress(const QString &title, int nProgress)
{
    if (nProgress == 0)
    {
        progressDialog = new QProgressDialog(title, "", 0, 100);
        progressDialog->setModal(false);
        progressDialog->setMinimumDuration(0);
        progressDialog->setCancelButton(0);
        progressDialog->setAutoClose(false);
        progressDialog->setValue(0);

        //fixme: (FUT) (MED) Minimum size
        progressDialog->setMinimumSize(300,100);
    }
    else if (nProgress == 100)
    {
        if (progressDialog)
        {
            progressDialog->close();
            progressDialog->deleteLater();
        }
    }
    else if (progressDialog)
        progressDialog->setValue(nProgress);
}

void WalletView::requestedSyncWarningInfo()
{
    Q_EMIT outOfSyncWarningClicked();
}
