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

#include "transactionview.h"

#include "addresstablemodel.h"
#include "units.h"
#include "csvmodelwriter.h"
#include "editaddressdialog.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "sendcoinsdialog.h"
#include "transactiondescdialog.h"
#include "transactionfilterproxy.h"
#include "transactionrecord.h"
#include "transactiontablemodel.h"
#include "walletmodel.h"
#include "richtextdelegate.h"

#include "ui_interface.h"

#include "alert.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPoint>
#include <QScrollBar>
#include <QSignalMapper>
#include <QTableView>
#include <QUrl>
#include <QVBoxLayout>

TransactionView::TransactionView(QWidget *parent)
: QWidget(parent)
, model(nullptr)
, transactionProxyModel(nullptr)
, transactionView(nullptr)
, abandonAction(nullptr)
, bumpFeeAction(nullptr)
, editLabelAction(nullptr)
, columnResizingFixer(nullptr)
{
    // Build filter row
    setContentsMargins(0,0,0,0);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0,0,0,0);

    //fixme: (FUT) (WATCH_ONLY)
    /*watchOnlyWidget = new QComboBox(this);
    watchOnlyWidget->setFixedWidth(24);
    watchOnlyWidget->addItem("", TransactionFilterProxy::WatchOnlyFilter_All);
    watchOnlyWidget->addItem(":/icons/eye_plus", "", TransactionFilterProxy::WatchOnlyFilter_Yes);
    watchOnlyWidget->addItem(":/icons/eye_minus", "", TransactionFilterProxy::WatchOnlyFilter_No);
    hlayout->addWidget(watchOnlyWidget);*/

    dateWidget = new QComboBox(this);
    dateWidget->addItem(tr("All dates"), All);
    dateWidget->addItem(tr("Today"), Today);
    dateWidget->addItem(tr("This week"), ThisWeek);
    dateWidget->addItem(tr("This month"), ThisMonth);
    dateWidget->addItem(tr("Last month"), LastMonth);
    dateWidget->addItem(tr("This year"), ThisYear);
    dateWidget->addItem(tr("Select range"), Range);
    hlayout->addWidget(dateWidget);

    typeWidget = new QComboBox(this);

    typeWidget->addItem(tr("All transaction types"), TransactionFilterProxy::ALL_TYPES);
    typeWidget->addItem(tr("Received with"), TransactionFilterProxy::TYPE(TransactionRecord::RecvWithAddress) |
                                        TransactionFilterProxy::TYPE(TransactionRecord::RecvFromOther));
    typeWidget->addItem(tr("Sent to"), TransactionFilterProxy::TYPE(TransactionRecord::SendToAddress) |
                                  TransactionFilterProxy::TYPE(TransactionRecord::SendToOther));
    typeWidget->addItem(tr("Internal transfer"), TransactionFilterProxy::TYPE(TransactionRecord::InternalTransfer));
    //typeWidget->addItem(tr("Internal account movement"), TransactionFilterProxy::TYPE(TransactionRecord::SendToSelf));
    typeWidget->addItem(tr("Generated"), TransactionFilterProxy::TYPE(TransactionRecord::Generated));
    typeWidget->addItem(tr("Witness reward"), TransactionFilterProxy::TYPE(TransactionRecord::GeneratedWitness));
    typeWidget->addItem(tr("Other"), TransactionFilterProxy::TYPE(TransactionRecord::Other));

    hlayout->addWidget(typeWidget);

    QLabel* searchLabel = new QLabel(GUIUtil::fontAwesomeSolid("\uf002"));
    searchLabel->setObjectName("searchLabel1");
    searchLabel->setTextFormat( Qt::RichText );
    searchLabel->setContentsMargins(0, 0, 0, 0);
    hlayout->addWidget(searchLabel);

    addressWidget = new QLineEdit(this);
    addressWidget->setObjectName("searchBox1");
#if QT_VERSION >= 0x040700
    addressWidget->setPlaceholderText(tr("Search label"));
#endif
    hlayout->addWidget(addressWidget);

    amountWidget = new QLineEdit(this);
    amountWidget->setObjectName("amountWidget");
    amountWidget->setAlignment(Qt::AlignRight);
#if QT_VERSION >= 0x040700
    amountWidget->setPlaceholderText(tr("Min amount"));
#endif
    amountWidget->setFixedWidth(120);

    amountWidget->setValidator(new QDoubleValidator(0, 1e20, 8, this));
    hlayout->addWidget(amountWidget);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setSpacing(0);

    QFrame* topMargin = new QFrame(this);
    topMargin->setObjectName("topMargin");
    topMargin->setContentsMargins(0,0,0,0);
    vlayout->addWidget(topMargin);

    vlayout->addWidget(createDateRangeWidget());

    QTableView *view = new QTableView(this);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(createDateRangeWidget());
    vlayout->addWidget(view);
    vlayout->setSpacing(0);

    // Always show scroll bar
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setTabKeyNavigation(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    view->installEventFilter(this);

    transactionView = view;
    transactionView->setObjectName("transactionView");

    // Actions
    clearOrphansAction = new QAction(tr("Clear orphan transactions"), this);
    abandonAction = new QAction(tr("Abandon transaction"), this);
    bumpFeeAction = new QAction(tr("Increase transaction fee"), this);
    bumpFeeAction->setObjectName("bumpFeeAction");
    QAction *copyAddressAction = new QAction(tr("Copy address"), this);
    copyAddressAction->setObjectName("action_transaction_view_copy_address");
    QAction *copyLabelAction = new QAction(tr("Copy label"), this);
    copyLabelAction->setObjectName("action_transaction_view_copy_label");
    QAction *copyAmountAction = new QAction(tr("Copy amount"), this);
    copyAmountAction->setObjectName("action_transaction_view_copy_amount");
    QAction *copyTxIDAction = new QAction(tr("Copy transaction ID"), this);
    copyTxIDAction->setObjectName("action_transaction_view_tx_id");
    QAction *copyTxHexAction = new QAction(tr("Copy raw transaction"), this);
    copyTxHexAction->setObjectName("action_transaction_view_raw_tx");
    QAction *copyTxPlainText = new QAction(tr("Copy full transaction details"), this);
    copyTxPlainText->setObjectName("action_transaction_view_tx_details_full");
    editLabelAction = new QAction(tr("Edit label"), this);
    editLabelAction->setObjectName("action_transaction_view_edit_label");
    QAction *showDetailsAction = new QAction(tr("Show transaction details"), this);
    showDetailsAction->setObjectName("action_transaction_view_show_details");

    contextMenu = new QMenu(this);
    contextMenu->setObjectName("contextMenu");
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    contextMenu->addAction(copyAmountAction);
    contextMenu->addAction(copyTxIDAction);
    contextMenu->addAction(copyTxHexAction);
    contextMenu->addAction(copyTxPlainText);
    contextMenu->addAction(showDetailsAction);
    contextMenu->addSeparator();
    contextMenu->addAction(bumpFeeAction);
    contextMenu->addAction(abandonAction);
    contextMenu->addAction(clearOrphansAction);
    contextMenu->addAction(editLabelAction);

    mapperThirdPartyTxUrls = new QSignalMapper(this);

    // Connect actions
    connect(mapperThirdPartyTxUrls, SIGNAL(mapped(QString)), this, SLOT(openThirdPartyTxUrl(QString)));

    connect(dateWidget, SIGNAL(activated(int)), this, SLOT(chooseDate(int)));
    connect(typeWidget, SIGNAL(activated(int)), this, SLOT(chooseType(int)));
    //connect(watchOnlyWidget, SIGNAL(activated(int)), this, SLOT(chooseWatchonly(int)));
    connect(addressWidget, SIGNAL(textChanged(QString)), this, SLOT(changedPrefix(QString)));
    connect(amountWidget, SIGNAL(textChanged(QString)), this, SLOT(changedAmount(QString)));

    connect(view, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(doubleClicked(QModelIndex)));
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));

    connect(bumpFeeAction, SIGNAL(triggered()), this, SLOT(bumpFee()));
    connect(abandonAction, SIGNAL(triggered()), this, SLOT(abandonTx()));
    connect(clearOrphansAction, SIGNAL(triggered()), this, SLOT(clearOrphanTransactions()));
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(copyLabel()));
    connect(copyAmountAction, SIGNAL(triggered()), this, SLOT(copyAmount()));
    connect(copyTxIDAction, SIGNAL(triggered()), this, SLOT(copyTxID()));
    connect(copyTxHexAction, SIGNAL(triggered()), this, SLOT(copyTxHex()));
    connect(copyTxPlainText, SIGNAL(triggered()), this, SLOT(copyTxPlainText()));
    connect(editLabelAction, SIGNAL(triggered()), this, SLOT(editLabel()));
    connect(showDetailsAction, SIGNAL(triggered()), this, SLOT(showDetails()));

    transactionView->setItemDelegate(new RichTextDelegate(this));
}

void TransactionView::setModel(WalletModel *_model)
{
    this->model = _model;
    if(_model)
    {
        connect(model, SIGNAL(activeAccountChanged(CAccount*)), this, SLOT(activeAccountChanged(CAccount*)));

        transactionProxyModel = new TransactionFilterProxy(this);
        transactionProxyModel->setSourceModel(_model->getTransactionTableModel());
        transactionProxyModel->setDynamicSortFilter(true);
        transactionProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        transactionProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        transactionProxyModel->setSortRole(Qt::EditRole);

        transactionView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        transactionView->setModel(transactionProxyModel);
        transactionView->setAlternatingRowColors(true);
        transactionView->setSelectionBehavior(QAbstractItemView::SelectRows);
        transactionView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        transactionView->setSortingEnabled(true);
        transactionView->sortByColumn(TransactionTableModel::Date, Qt::DescendingOrder);
        transactionView->verticalHeader()->hide();
        transactionView->setShowGrid(false);

        transactionView->setColumnWidth(TransactionTableModel::Status, STATUS_COLUMN_WIDTH);
        transactionView->setColumnWidth(TransactionTableModel::Watchonly, WATCHONLY_COLUMN_WIDTH);
        transactionView->setColumnHidden(TransactionTableModel::Watchonly, true);
        transactionView->setColumnWidth(TransactionTableModel::Date, DATE_COLUMN_WIDTH);
        transactionView->setColumnWidth(TransactionTableModel::Type, TYPE_COLUMN_WIDTH);
        transactionView->setColumnHidden(TransactionTableModel::Type, true);
        transactionView->setColumnWidth(TransactionTableModel::AmountSent, AMOUNT_MINIMUM_COLUMN_WIDTH);
        transactionView->setColumnWidth(TransactionTableModel::AmountReceived, AMOUNT_MINIMUM_COLUMN_WIDTH);

        // Description column streches, all other columns fixed but user resizable.
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::Status, QHeaderView::Interactive);
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::Watchonly, QHeaderView::Interactive);
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::Date, QHeaderView::Interactive);
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::Type, QHeaderView::Interactive);
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::ToAddress, QHeaderView::Stretch);
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::AmountReceived, QHeaderView::Interactive);
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::AmountSent, QHeaderView::Interactive);

        /*columnResizingFixer = new GUIUtil::TableViewLastColumnResizingFixer(transactionView, AMOUNT_MINIMUM_COLUMN_WIDTH, MINIMUM_COLUMN_WIDTH, this);*/

        if (_model->getOptionsModel())
        {
            // Add third party transaction URLs to context menu
            QStringList listUrls = _model->getOptionsModel()->getThirdPartyTxUrls().split("|", QString::SkipEmptyParts);
            for (int i = 0; i < listUrls.size(); ++i)
            {
                QString host = QUrl(listUrls[i].trimmed(), QUrl::StrictMode).host();
                if (!host.isEmpty())
                {
                    QAction *thirdPartyTxUrlAction = new QAction(host, this); // use host as menu item label
                    thirdPartyTxUrlAction->setObjectName("action_transaction_view_third_party_url");
                    if (i == 0)
                        contextMenu->addSeparator();
                    contextMenu->addAction(thirdPartyTxUrlAction);
                    connect(thirdPartyTxUrlAction, SIGNAL(triggered()), mapperThirdPartyTxUrls, SLOT(map()));
                    mapperThirdPartyTxUrls->setMapping(thirdPartyTxUrlAction, listUrls[i].trimmed());
                }
            }
        }

        // show/hide column Watch-only
        updateWatchOnlyColumn(_model->haveWatchOnly());

        // Watch-only signal
        connect(_model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyColumn(bool)));

        activeAccountChanged(model->getActiveAccount());
    }
}

void TransactionView::chooseDate(int idx)
{
    if(!transactionProxyModel)
        return;
    QDate current = QDate::currentDate();
    dateRangeWidget->setVisible(false);
    switch(dateWidget->itemData(idx).toInt())
    {
    case All:
        transactionProxyModel->setDateRange(
                TransactionFilterProxy::MIN_DATE,
                TransactionFilterProxy::MAX_DATE);
        break;
    case Today:
        transactionProxyModel->setDateRange(
                QDateTime(current),
                TransactionFilterProxy::MAX_DATE);
        break;
    case ThisWeek: {
        // Find last Monday
        QDate startOfWeek = current.addDays(-(current.dayOfWeek()-1));
        transactionProxyModel->setDateRange(
                QDateTime(startOfWeek),
                TransactionFilterProxy::MAX_DATE);

        } break;
    case ThisMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month(), 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case LastMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month(), 1).addMonths(-1)),
                QDateTime(QDate(current.year(), current.month(), 1)));
        break;
    case ThisYear:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), 1, 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case Range:
        dateRangeWidget->setVisible(true);
        dateRangeChanged();
        break;
    }
}

void TransactionView::chooseType(int idx)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setTypeFilter(typeWidget->itemData(idx).toInt());
}

void TransactionView::toggleShowOrphans(bool showOrphans)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setShowOrphaned(showOrphans);
}

void TransactionView::chooseWatchonly(int idx)
{
    if(!transactionProxyModel)
        return;
    //transactionProxyModel->setWatchOnlyFilter((TransactionFilterProxy::WatchOnlyFilter)watchOnlyWidget->itemData(idx).toInt());
}

void TransactionView::changedPrefix(const QString &prefix)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setAddressPrefix(prefix);
}

void TransactionView::changedAmount(const QString &amount)
{
    if(!transactionProxyModel)
        return;
    CAmount amount_parsed = 0;
    if(GuldenUnits::parse(model->getOptionsModel()->getDisplayUnit(), amount, &amount_parsed))
    {
        transactionProxyModel->setMinAmount(amount_parsed);
    }
    else
    {
        transactionProxyModel->setMinAmount(0);
    }
}

void TransactionView::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(this,
        tr("Export Transaction History"), QString(),
        tr("Comma separated file (*.csv)"), NULL);

    if (filename.isNull())
        return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(transactionProxyModel);
    writer.addColumn(tr("Confirmed"), 0, TransactionTableModel::ConfirmedRole);
    //if (model && model->haveWatchOnly())
        //writer.addColumn(tr("Watch-only"), TransactionTableModel::Watchonly);
    writer.addColumn(tr("Date"), 0, TransactionTableModel::DateRole);
    writer.addColumn(tr("Type"), TransactionTableModel::Type, Qt::EditRole);
    writer.addColumn(tr("Label"), 0, TransactionTableModel::LabelRole);
    writer.addColumn(tr("Address"), 0, TransactionTableModel::AddressRole);
    writer.addColumn(tr("Credit"), 0, TransactionTableModel::FormattedAmountReceivedRole);
    writer.addColumn(tr("Debit"), 0, TransactionTableModel::FormattedAmountSentRole);
    writer.addColumn(tr("ID"), 0, TransactionTableModel::TxIDRole);

    if(!writer.write()) {
        Q_EMIT message(tr("Exporting Failed"), tr("There was an error trying to save the transaction history to %1.").arg(filename),
            CClientUIInterface::MSG_ERROR);
    }
    else {
        Q_EMIT message(tr("Exporting Successful"), tr("The transaction history was successfully saved to %1.").arg(filename),
            CClientUIInterface::MSG_INFORMATION);
    }
}

void TransactionView::activeAccountChanged(CAccount* account)
{
    LogPrintf("TransactionView::activeAccountChanged\n");

    if(!transactionProxyModel)
        return;
    transactionProxyModel->setAccountFilter(account);
}

void TransactionView::contextualMenu(const QPoint &point)
{
    QModelIndex index = transactionView->indexAt(point);
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);
    if (selection.empty())
        return;

    //fixme: (FUT) relook at all right click items wholistically
    bool canEditLabel = true;
    int transactionType = selection.at(0).data(TransactionTableModel::TypeRole).toInt();
    int transactionStatus = selection.at(0).data(TransactionTableModel::StatusRole).toInt();
    switch (transactionType)
    {
        case TransactionRecord::Other:
        case TransactionRecord::SendToAddress:
        case TransactionRecord::SendToOther:
        case TransactionRecord::RecvWithAddress:
        case TransactionRecord::RecvFromOther:
        case TransactionRecord::SendToSelf:
        case TransactionRecord::InternalTransfer:
            break;
        case TransactionRecord::WitnessRenew:
        case TransactionRecord::WitnessFundSend:
        case TransactionRecord::WitnessEmptySend:
        case TransactionRecord::WitnessIncreaseSend:
        case TransactionRecord::WitnessFundRecv:
        case TransactionRecord::WitnessIncreaseRecv:
        case TransactionRecord::WitnessChangeKeyRecv:
        case TransactionRecord::WitnessEmptyRecv:
        case TransactionRecord::WitnessRearrangeRecv:
        case TransactionRecord::Generated:
        case TransactionRecord::GeneratedWitness:
            canEditLabel = false;
            break;
    }
    editLabelAction->setEnabled(canEditLabel);

    bool isOrphanTransaction = false;
    if (transactionStatus == TransactionStatus::NotAccepted || transactionStatus == TransactionStatus::Abandoned)
    {
        if(transactionType == TransactionRecord::GeneratedWitness || transactionType == TransactionRecord::Generated)
        {
            isOrphanTransaction = true;
        }
    }
                
    // check if transaction can be abandoned, disable context menu action in case it doesn't
    uint256 hash;
    hash.SetHex(selection.at(0).data(TransactionTableModel::TxHashRole).toString().toStdString());
    abandonAction->setEnabled(model->transactionCanBeAbandoned(hash));
    abandonAction->setVisible(model->transactionCanBeAbandoned(hash));
    clearOrphansAction->setVisible(isOrphanTransaction);
    bumpFeeAction->setEnabled(model->transactionCanBeBumped(hash));
    bumpFeeAction->setVisible(model->transactionCanBeBumped(hash));

    if(index.isValid())
    {
        contextMenu->popup(transactionView->viewport()->mapToGlobal(point));
    }
}

void TransactionView::abandonTx()
{
    if(!transactionView || !transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);

    // get the hash from the TxHashRole (QVariant / QString)
    uint256 hash;
    QString hashQStr = selection.at(0).data(TransactionTableModel::TxHashRole).toString();
    hash.SetHex(hashQStr.toStdString());

    // Abandon the wallet transaction over the walletModel
    model->abandonTransaction(hash);

    // Update the table
    model->getTransactionTableModel()->updateTransaction(hashQStr, CT_UPDATED, false);
}

void TransactionView::clearOrphanTransactions()
{
    LogPrintf("TransactionView::clearOrphanTransactions - Purging orphan transactions for account\n");
    if(!pactiveWallet || !transactionView || !transactionView->selectionModel())
        return;
    
    LOCK(pactiveWallet->cs_wallet);
    
    std::vector<uint256> transactionsToZap;
    std::vector<uint256> transactionsZapped;
    int row = transactionProxyModel->rowCount();
    for (int i = 0; i < row ; ++i)
    {
        int type = transactionProxyModel->data(transactionProxyModel->index(i, 0), TransactionTableModel::TypeRole).toInt();
        int status = transactionProxyModel->data(transactionProxyModel->index(i, 0), TransactionTableModel::StatusRole).toInt();
        if (status == TransactionStatus::NotAccepted || status == TransactionStatus::Abandoned)
        {
            if(type == TransactionRecord::GeneratedWitness || type == TransactionRecord::Generated)
            {
                uint256 hash;
                QString hashQStr = transactionProxyModel->data(transactionProxyModel->index(i, 0), TransactionTableModel::TxHashRole).toString();
                hash.SetHex(hashQStr.toStdString());
                transactionsToZap.emplace_back(hash);
            }
        }
    }
    if (transactionsToZap.size() > 0)
    {
        LogPrintf("TransactionView::clearOrphanTransactions - Purging [%d] transactions\n", transactionsToZap.size());
        
        CWalletDB walletdb(*pactiveWallet->dbw);
        if (pactiveWallet->ZapSelectTx(walletdb, transactionsToZap, transactionsZapped) != DB_LOAD_OK)
        {
            std::string strErrorMessage = "Failed to erase orphan transactions for account.\n";
            LogPrintf("%s", strErrorMessage.c_str());
            CAlert::Notify(strErrorMessage, true, true);
        }
        transactionProxyModel->invalidate();
    }
}

void TransactionView::bumpFee()
{
    if(!transactionView || !transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);

    // get the hash from the TxHashRole (QVariant / QString)
    uint256 hash;
    QString hashQStr = selection.at(0).data(TransactionTableModel::TxHashRole).toString();
    hash.SetHex(hashQStr.toStdString());

    // Bump tx fee over the walletModel
    if (model->bumpFee(hash)) {
        // Update the table
        model->getTransactionTableModel()->updateTransaction(hashQStr, CT_UPDATED, true);
    }
}

void TransactionView::copyAddress()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::AddressRole);
}

void TransactionView::copyLabel()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::LabelRole);
}

void TransactionView::copyAmount()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::FormattedAmountRole);
}

void TransactionView::copyTxID()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxIDRole);
}

void TransactionView::copyTxHex()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxHexRole);
}

void TransactionView::copyTxPlainText()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxPlainTextRole);
}

void TransactionView::editLabel()
{
    if(!transactionView->selectionModel() ||!model)
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        AddressTableModel *addressBook = model->getAddressTableModel();
        if(!addressBook)
            return;
        QString address = selection.at(0).data(TransactionTableModel::AddressRole).toString();
        if(address.isEmpty())
        {
            // If this transaction has no associated address, exit
            return;
        }
        // Is address in address book? Address book can miss address when a transaction is
        // sent from outside the UI.
        int idx = addressBook->lookupAddress(address);
        if(idx != -1)
        {
            // Edit sending / receiving address
            QModelIndex modelIdx = addressBook->index(idx, 0, QModelIndex());
            // Determine type of address, launch appropriate editor dialog type
            QString type = modelIdx.data(AddressTableModel::TypeRole).toString();

            EditAddressDialog dlg(
                type == AddressTableModel::Receive
                ? EditAddressDialog::EditReceivingAddress
                : EditAddressDialog::EditSendingAddress, this);
            dlg.setModel(addressBook);
            dlg.loadRow(idx);
            dlg.exec();
        }
        else
        {
            // Add sending address
            EditAddressDialog dlg(EditAddressDialog::NewSendingAddress,
                this);
            dlg.setModel(addressBook);
            dlg.setAddress(address);
            dlg.exec();
        }
    }
}

void TransactionView::showDetails()
{
    if(!transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        TransactionDescDialog *dlg = new TransactionDescDialog(selection.at(0));
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    }
}

void TransactionView::openThirdPartyTxUrl(QString url)
{
    if(!transactionView || !transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);
    if(!selection.isEmpty())
         QDesktopServices::openUrl(QUrl::fromUserInput(url.replace("%s", selection.at(0).data(TransactionTableModel::TxHashRole).toString())));
}

QWidget *TransactionView::createDateRangeWidget()
{
    dateRangeWidget = new QFrame();
    dateRangeWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    dateRangeWidget->setContentsMargins(1,1,1,1);
    QHBoxLayout *layout = new QHBoxLayout(dateRangeWidget);
    layout->setContentsMargins(0,0,0,0);
    layout->addSpacing(23);
    layout->addWidget(new QLabel(tr("Range:")));

    dateFrom = new QDateTimeEdit(this);
    dateFrom->setDisplayFormat("dd/MM/yy");
    dateFrom->setCalendarPopup(true);
    dateFrom->setMinimumWidth(100);
    dateFrom->setDate(QDate::currentDate().addDays(-7));
    layout->addWidget(dateFrom);
    layout->addWidget(new QLabel(tr("to")));

    dateTo = new QDateTimeEdit(this);
    dateTo->setDisplayFormat("dd/MM/yy");
    dateTo->setCalendarPopup(true);
    dateTo->setMinimumWidth(100);
    dateTo->setDate(QDate::currentDate());
    layout->addWidget(dateTo);
    layout->addStretch();

    // Hide by default
    dateRangeWidget->setVisible(false);

    // Notify on change
    connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));
    connect(dateTo, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));

    return dateRangeWidget;
}

void TransactionView::dateRangeChanged()
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setDateRange(
            QDateTime(dateFrom->date()),
            QDateTime(dateTo->date()).addDays(1));
}

void TransactionView::focusTransaction(const QModelIndex &idx)
{
    if(!transactionProxyModel)
        return;
    QModelIndex targetIdx = transactionProxyModel->mapFromSource(idx);
    transactionView->scrollTo(targetIdx);
    transactionView->setCurrentIndex(targetIdx);
    transactionView->setFocus();
}

// We override the virtual resizeEvent of the QWidget to adjust tables column
// sizes as the tables width is proportional to the dialogs width.
void TransactionView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    //columnResizingFixer->stretchColumnWidth(TransactionTableModel::ToAddress - 1);
}

// Need to override default Ctrl+C action for amount as default behaviour is just to copy DisplayRole text
bool TransactionView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_C && ke->modifiers().testFlag(Qt::ControlModifier))
        {
             GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxPlainTextRole);
             return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// show/hide column Watch-only
void TransactionView::updateWatchOnlyColumn(bool fHaveWatchOnly)
{
    //watchOnlyWidget->setVisible(fHaveWatchOnly);
    //transactionView->setColumnHidden(TransactionTableModel::Watchonly, !fHaveWatchOnly);
    transactionView->setColumnHidden(TransactionTableModel::Watchonly, true);
}
