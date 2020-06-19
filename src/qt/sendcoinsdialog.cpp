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

#include "sendcoinsdialog.h"
#include "ui_sendcoinsdialog.h"

#include "addresstablemodel.h"
#include "units.h"
#include "clientmodel.h"
#include "coincontroldialog.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "guldensendcoinsentry.h"
#include "nocksrequest.h"
#include "gui.h"
#include "walletmodel.h"

#include "base58.h"
#include "chainparams.h"
#include "wallet/coincontrol.h"
#include "validation/validation.h" // mempool and minRelayTxFee
#include "ui_interface.h"
#include "txmempool.h"
#include "policy/fees.h"
#include "wallet/wallet.h"

#include <QFontMetrics>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QTextDocument>
#include <QTimer>

SendCoinsDialog::SendCoinsDialog(const QStyle *_platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendCoinsDialog),
    clientModel(0),
    model(0),
    fNewRecipientAllowed(true),
    fFeeMinimized(true),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

    GUIUtil::setupAddressWidget(ui->lineEditCoinControlChange, this);


    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addEntry()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    // Coin Control
    connect(ui->pushButtonCoinControl, SIGNAL(clicked()), this, SLOT(coinControlButtonClicked()));
    connect(ui->checkBoxCoinControlChange, SIGNAL(stateChanged(int)), this, SLOT(coinControlChangeChecked(int)));
    connect(ui->lineEditCoinControlChange, SIGNAL(textEdited(const QString &)), this, SLOT(coinControlChangeEdited(const QString &)));

    // Coin Control: clipboard actions
    QAction *clipboardQuantityAction = new QAction(tr("Copy quantity"), this);
    clipboardQuantityAction->setObjectName("action_sendcoins_clipboarc_copy_quantity");
    QAction *clipboardAmountAction = new QAction(tr("Copy amount"), this);
    clipboardAmountAction->setObjectName("action_sendcoins_clipboarc_copy_amount");
    QAction *clipboardFeeAction = new QAction(tr("Copy fee"), this);
    clipboardFeeAction->setObjectName("action_sendcoins_clipboarc_copy_fee");
    QAction *clipboardAfterFeeAction = new QAction(tr("Copy after fee"), this);
    clipboardAfterFeeAction->setObjectName("action_sendcoins_clipboarc_copy_after_fee");
    QAction *clipboardBytesAction = new QAction(tr("Copy bytes"), this);
    clipboardBytesAction->setObjectName("action_sendcoins_clipboarc_copy_bytes");
    QAction *clipboardLowOutputAction = new QAction(tr("Copy dust"), this);
    clipboardLowOutputAction->setObjectName("action_sendcoins_clipboarc_copy_dust");
    QAction *clipboardChangeAction = new QAction(tr("Copy change"), this);
    clipboardChangeAction->setObjectName("action_sendcoins_clipboarc_copy_change");
    connect(clipboardQuantityAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardQuantity()));
    connect(clipboardAmountAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAmount()));
    connect(clipboardFeeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardFee()));
    connect(clipboardAfterFeeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAfterFee()));
    connect(clipboardBytesAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardBytes()));
    connect(clipboardLowOutputAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardLowOutput()));
    connect(clipboardChangeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardChange()));
    ui->labelCoinControlQuantity->addAction(clipboardQuantityAction);
    ui->labelCoinControlAmount->addAction(clipboardAmountAction);
    ui->labelCoinControlFee->addAction(clipboardFeeAction);
    ui->labelCoinControlAfterFee->addAction(clipboardAfterFeeAction);
    ui->labelCoinControlBytes->addAction(clipboardBytesAction);
    ui->labelCoinControlLowOutput->addAction(clipboardLowOutputAction);
    ui->labelCoinControlChange->addAction(clipboardChangeAction);

    // init transaction fee section
    QSettings settings;
    if (!settings.contains("fFeeSectionMinimized"))
        settings.setValue("fFeeSectionMinimized", true);
    if (!settings.contains("nFeeRadio") && settings.contains("nTransactionFee") && settings.value("nTransactionFee").toLongLong() > 0) // compatibility
        settings.setValue("nFeeRadio", 1); // custom
    if (!settings.contains("nFeeRadio"))
        settings.setValue("nFeeRadio", 0); // recommended
    if (!settings.contains("nCustomFeeRadio") && settings.contains("nTransactionFee") && settings.value("nTransactionFee").toLongLong() > 0) // compatibility
        settings.setValue("nCustomFeeRadio", 1); // total at least
    if (!settings.contains("nCustomFeeRadio"))
        settings.setValue("nCustomFeeRadio", 0); // per kilobyte
    if (!settings.contains("nSmartFeeSliderPosition"))
        settings.setValue("nSmartFeeSliderPosition", 0);
    if (!settings.contains("nTransactionFee"))
        settings.setValue("nTransactionFee", (qint64)DEFAULT_TRANSACTION_FEE);
    if (!settings.contains("fPayOnlyMinFee"))
        settings.setValue("fPayOnlyMinFee", false);
    ui->groupFee->setId(ui->radioSmartFee, 0);
    ui->groupFee->setId(ui->radioCustomFee, 1);
    ui->groupFee->button((int)std::max(0, std::min(1, settings.value("nFeeRadio").toInt())))->setChecked(true);
    ui->groupCustomFee->setId(ui->radioCustomPerKilobyte, 0);
    //ui->groupCustomFee->button((int)std::max(0, std::min(1, settings.value("nCustomFeeRadio").toInt())))->setChecked(true);
    ui->customFee->setAmount(settings.value("nTransactionFee").toLongLong());
    ui->checkBoxMinimumFee->setChecked(settings.value("fPayOnlyMinFee").toBool());
    minimizeFeeSection(settings.value("fFeeSectionMinimized").toBool());

    //Gulden
    ui->customFee->setVisible(false);
    ui->labelFeeHeadline->setVisible(false);
    ui->label->setVisible(false);
    ui->labelBalance->setVisible(false);
    ui->addButton->setVisible(false);
    ui->frameFee->setVisible(false);

    ui->horizontalLayout->removeWidget(ui->sendButton);
    QPushButton* editButton = new QPushButton();
    editButton->setText(tr("&Edit"));
    editButton->setObjectName("editButton");
    ui->horizontalLayout->addWidget(editButton);
    ui->horizontalLayout->addWidget(ui->sendButton);

    ui->sendButton->setIcon(QIcon());
    ui->sendButton->setText(tr("&Send"));

    ui->clearButton->setIcon(QIcon());
    ui->clearButton->setText(tr("&Clear"));
    ui->clearButton->setMinimumSize(QSize(0, 0));
    ui->sendButton->setMinimumSize(QSize(0, 0));


    ui->verticalLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    QFrame* horizontalLine = new QFrame(this);
    horizontalLine->setObjectName("horizontalLine");
    horizontalLine->setFrameStyle(QFrame::HLine);
    horizontalLine->setFixedHeight(1);
    horizontalLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    ui->verticalLayout->insertWidget(2, horizontalLine);

    QPushButton* deleteButton = new QPushButton();
    deleteButton->setText(tr("&Delete"));
    deleteButton->setObjectName("deleteButton");
    ui->horizontalLayout->insertWidget(0, deleteButton);

    ui->clearButton->setCursor(Qt::PointingHandCursor);
    ui->sendButton->setCursor(Qt::PointingHandCursor);
    editButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setCursor(Qt::PointingHandCursor);

    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteAddressBookEntry()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editAddressBookEntry()));

    addEntry();

    nocksRequest = NULL;
}

void SendCoinsDialog::setClientModel(ClientModel *_clientModel)
{
    this->clientModel = _clientModel;

    if (_clientModel) {
        connect(_clientModel, SIGNAL(numBlocksChanged(int,QDateTime,double)), this, SLOT(updateSmartFeeLabel()));
    }
}

void SendCoinsDialog::setModel(WalletModel *_model)
{
    this->model = _model;

    if(_model && _model->getOptionsModel())
    {
        for(int i = 0; i < ui->entries->count(); ++i)
        {
            GuldenSendCoinsEntry *entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(i)->widget());
            if(entry)
            {
                entry->setModel(_model);
            }
        }

        setBalance(_model->getBalances(), _model->getWatchBalance(), _model->getWatchUnconfirmedBalance(), _model->getWatchImmatureBalance());
        connect(_model, SIGNAL(balanceChanged(WalletBalances, CAmount, CAmount, CAmount)), this, SLOT(setBalance(WalletBalances, CAmount, CAmount, CAmount)));
        connect(_model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
        updateDisplayUnit();

        // Coin Control
        connect(_model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(coinControlUpdateLabels()));
        connect(_model->getOptionsModel(), SIGNAL(coinControlFeaturesChanged(bool)), this, SLOT(coinControlFeatureChanged(bool)));
        ui->frameCoinControl->setVisible(_model->getOptionsModel()->getCoinControlFeatures());
        coinControlUpdateLabels();

        // fee section
        connect(ui->sliderSmartFee, SIGNAL(valueChanged(int)), this, SLOT(updateSmartFeeLabel()));
        connect(ui->sliderSmartFee, SIGNAL(valueChanged(int)), this, SLOT(updateGlobalFeeVariables()));
        connect(ui->sliderSmartFee, SIGNAL(valueChanged(int)), this, SLOT(coinControlUpdateLabels()));
        connect(ui->groupFee, SIGNAL(buttonClicked(int)), this, SLOT(updateFeeSectionControls()));
        connect(ui->groupFee, SIGNAL(buttonClicked(int)), this, SLOT(updateGlobalFeeVariables()));
        connect(ui->groupFee, SIGNAL(buttonClicked(int)), this, SLOT(coinControlUpdateLabels()));
        connect(ui->groupCustomFee, SIGNAL(buttonClicked(int)), this, SLOT(updateGlobalFeeVariables()));
        connect(ui->groupCustomFee, SIGNAL(buttonClicked(int)), this, SLOT(coinControlUpdateLabels()));
        connect(ui->customFee, SIGNAL(amountChanged()), this, SLOT(updateGlobalFeeVariables()));
        connect(ui->customFee, SIGNAL(amountChanged()), this, SLOT(coinControlUpdateLabels()));
        connect(ui->checkBoxMinimumFee, SIGNAL(stateChanged(int)), this, SLOT(setMinimumFee()));
        connect(ui->checkBoxMinimumFee, SIGNAL(stateChanged(int)), this, SLOT(updateFeeSectionControls()));
        connect(ui->checkBoxMinimumFee, SIGNAL(stateChanged(int)), this, SLOT(updateGlobalFeeVariables()));
        connect(ui->checkBoxMinimumFee, SIGNAL(stateChanged(int)), this, SLOT(coinControlUpdateLabels()));
        ui->customFee->setSingleStep(CWallet::GetRequiredFee(1000));
        updateFeeSectionControls();
        updateMinFeeLabel();
        updateSmartFeeLabel();
        updateGlobalFeeVariables();

        // set default rbf checkbox state
        ui->optInRBF->setCheckState(model->getDefaultWalletRbf() ? Qt::Checked : Qt::Unchecked);

        // set the smartfee-sliders default value (wallets default conf.target or last stored value)
        QSettings settings;
        if (settings.value("nSmartFeeSliderPosition").toInt() == 0)
            ui->sliderSmartFee->setValue(ui->sliderSmartFee->maximum() - model->getDefaultConfirmTarget() + 2);
        else
            ui->sliderSmartFee->setValue(settings.value("nSmartFeeSliderPosition").toInt());
    }
}

SendCoinsDialog::~SendCoinsDialog()
{
    QSettings settings;
    settings.setValue("fFeeSectionMinimized", fFeeMinimized);
    settings.setValue("nFeeRadio", ui->groupFee->checkedId());
    settings.setValue("nCustomFeeRadio", ui->groupCustomFee->checkedId());
    settings.setValue("nSmartFeeSliderPosition", ui->sliderSmartFee->value());
    settings.setValue("nTransactionFee", (qint64)ui->customFee->amount());
    settings.setValue("fPayOnlyMinFee", ui->checkBoxMinimumFee->isChecked());

    delete ui;

    if (nocksRequest)
    {
        nocksRequest->deleteLater();
        nocksRequest = NULL;
    }
}

void SendCoinsDialog::setAmount(CAmount amount)
{
    if (ui->entries->count() > 0)
    {
        GuldenSendCoinsEntry* entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(0)->widget());
        entry->setAmount(amount);
    }
}

void SendCoinsDialog::on_sendButton_clicked()
{
    ui->sendButton->setEnabled(true);

    if(!model || !model->getOptionsModel())
        return;

    QList<SendCoinsRecipient> recipients;
    bool valid = true;

    if (!pendingRecipients.isEmpty())
    {
        recipients = pendingRecipients;
    }
    else
    {
        for(int i = 0; i < ui->entries->count(); ++i)
        {
            GuldenSendCoinsEntry *entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(i)->widget());
            if(entry)
            {
                if(entry->validate())
                {
                    recipients.append(entry->getValue());
                }
                else
                {
                    valid = false;
                }
            }
        }
    }

    if(!valid || recipients.isEmpty())
    {
        return;
    }

    if (nocksRequest)
    {
        nocksRequest->deleteLater();
        nocksRequest = NULL;
    }

    // Convert all our 'forex' requests into normal requests before commencing.
    pendingRecipients = recipients;
    for (int i=0; i < pendingRecipients.size(); ++i)
    {
        if (pendingRecipients[i].paymentType != SendCoinsRecipient::PaymentType::NormalPayment)
        {
            nocksRequest = new NocksRequest(this);
            connect(nocksRequest, SIGNAL(requestProcessed()), this, SLOT(on_sendButton_clicked()));
            nocksRequest->startRequest(&pendingRecipients[i], NocksRequest::RequestType::Order, "NLG", "EUR");
            ui->sendButton->setEnabled(false);
            return;
        }
        else if(!pendingRecipients[i].forexFailCode.empty())
        {
            WalletModel::SendCoinsReturn errorRet;
            errorRet.status = WalletModel::StatusCode::ForexFailed;
            processSendCoinsReturn(errorRet, QString::fromStdString(pendingRecipients[i].forexFailCode));
            ui->sendButton->setEnabled(true);
            pendingRecipients.clear();
            return;
        }
    }
    pendingRecipients.clear();


    fNewRecipientAllowed = false;
    WalletModel::UnlockContext ctx(model->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        fNewRecipientAllowed = true;
        return;
    }

    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;

    // Always use a CCoinControl instance, use the CoinControlDialog instance if CoinControl has been enabled
    CCoinControl ctrl;

    //fixme: (FUT) (COIN_CONTROL)
    #if 0
    if (model->getOptionsModel()->getCoinControlFeatures())
        ctrl = *CoinControlDialog::coinControl;
    if (ui->radioSmartFee->isChecked())
        ctrl.nConfirmTarget = ui->sliderSmartFee->maximum() - ui->sliderSmartFee->value() + 2;
    #endif


    CAccount* forAccount = model->getActiveAccount();
    std::vector<CAccount*> accountsToTry;
    {
        LOCK(pactiveWallet->cs_wallet);

        //Try to pay first from legacy accounts (empty them)
        for ( const auto& accountPair : pactiveWallet->mapAccounts )
        {
            if(accountPair.second->getParentUUID() == forAccount->getUUID())
            {
                accountsToTry.push_back(accountPair.second);
            }
        }
        accountsToTry.push_back(forAccount);
    }
    bool allFailed=true;
    bool didShowError=false;
    for (auto& account : accountsToTry)
    {
        //fixme: (FUT) Check if 'spend unconfirmed' is checked or not.
        if ((model->getBalance(account) + model->getUnconfirmedBalance(account)) >= currentTransaction.getTotalTransactionAmount())
        {
            prepareStatus = model->prepareTransaction(account, currentTransaction, &ctrl);
            if (prepareStatus.status == WalletModel::OK)
            {
                allFailed = false;
                didShowError = false;
                forAccount = account;
                break;
            }
            if (prepareStatus.status == WalletModel::PoW2NotActive)
            {
                processSendCoinsReturn(prepareStatus);
            }
            if (prepareStatus.status != WalletModel::AmountExceedsBalance)
            {
                didShowError = true;
            }
        }
    }
    if (allFailed)
    {
        // If we failed inside prepareTransaction then we already displayed a warning, so don't display another one.
        if (didShowError)
        {
            return;
        }
        prepareStatus.status =  WalletModel::AmountExceedsBalance;
    }

    //fixme: (FUT) (RBF)
    #if 0
    ctrl.signalRbf = ui->optInRBF->isChecked();
    #endif

    // process prepareStatus and on error generate message shown to user
    processSendCoinsReturn(prepareStatus, GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));

    if(prepareStatus.status != WalletModel::OK) {
        fNewRecipientAllowed = true;
        return;
    }

    CAmount txFee = currentTransaction.getTransactionFee();

    // Format confirmation message
    QStringList formatted;
    for(const SendCoinsRecipient &rcp : currentTransaction.getRecipients())
    {
        // generate bold amount string
        QString amount = "<b>" + GuldenUnits::formatHtmlWithUnit(model->getOptionsModel()->getDisplayUnit(), rcp.amount);
        amount.append("</b>");
        // generate monospace address string
        QString address = "<span style='font-family: monospace;'>" + rcp.address;
        address.append("</span>");

        if (!rcp.forexAddress.isEmpty())
        {
            amount = "<b>";
            if (rcp.forexPaymentType == SendCoinsRecipient::PaymentType::IBANPayment)
            {
                amount.append(GuldenUnits::format(GuldenUnits::NLG, rcp.forexAmount, false, GuldenUnits::separatorAlways, 2));
                amount.append(" Euro");
            }
            else if(rcp.forexPaymentType == SendCoinsRecipient::PaymentType::BitcoinPayment)
            {
                amount.append(GuldenUnits::format(GuldenUnits::NLG, rcp.forexAmount));
                amount.append(" BTC");
            }
            amount.append(" (");
            amount.append(GuldenUnits::formatHtmlWithUnit(model->getOptionsModel()->getDisplayUnit(), rcp.amount));
            amount.append(")</b>");

            address = "<span style='font-family: monospace;'>" + rcp.forexAddress;
            address.append("</span>");
        }

        QString recipientElement;

        if (!rcp.paymentRequest.IsInitialized()) // normal payment
        {
            if(rcp.label.length() > 0) // label with address
            {
                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
                recipientElement.append(QString(" (%1)").arg(address));
            }
            else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
        }
        else if(!rcp.authenticatedMerchant.isEmpty()) // authenticated payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
        }
        else // unauthenticated payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, address);
        }

        formatted.append(recipientElement);
    }

    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />%1");

    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr />");
        questionString.append(GuldenUnits::format(GuldenUnits::NLG, txFee, false, GuldenUnits::separatorAlways));
        questionString.append(" NLG ");
        questionString.append(tr("added as transaction fee"));

        // append transaction size
        questionString.append(" (" + QString::number((double)currentTransaction.getTransactionSize() / 1000) + " kB)");
    }

    // add total amount
    questionString.append("<hr />");
    CAmount totalAmount = currentTransaction.getTotalTransactionAmount() + txFee;
    questionString.append(tr("Total Amount"));
    questionString.append(QString(" <span style='color:#aa0000;'>%1</span>").arg(GuldenUnits::formatHtmlWithUnit(model->getOptionsModel()->getDisplayUnit(), totalAmount)));

    if (ui->optInRBF->isChecked())
    {
        questionString.append("<hr /><span>");
        questionString.append(tr("This transaction signals replaceability (optin-RBF)."));
        questionString.append("</span>");
    }


    QDialog* d = GUI::createDialog(this, questionString.arg(formatted.join("<br />")), tr("Send"), tr("Cancel"), 600, 360, "SendConfirmationDialog");

    int result = d->exec();
    if(result != QDialog::Accepted)
    {
        fNewRecipientAllowed = true;
        return;
    }

    for(const SendCoinsRecipient &rcp : currentTransaction.getRecipients())
    {
        if(!rcp.forexAddress.isEmpty() && GetTime() > rcp.expiry)
        {
            prepareStatus.status =  WalletModel::PaymentRequestExpired;
            processSendCoinsReturn(prepareStatus, GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));
            fNewRecipientAllowed = true;
            return;
        }
    }

    // now send the prepared transaction
    WalletModel::SendCoinsReturn sendStatus = model->sendCoins(currentTransaction);
    // process sendStatus and on error generate message shown to user
    processSendCoinsReturn(sendStatus);

    if (sendStatus.status == WalletModel::OK)
    {
        accept();
        CoinControlDialog::coinControl->UnSelectAll();
        coinControlUpdateLabels();
    }
    fNewRecipientAllowed = true;
}

void SendCoinsDialog::clear()
{
    // Remove entries until only one left
    while(ui->entries->count())
    {
        ui->entries->takeAt(0)->widget()->deleteLater();
    }
    addEntry();

    updateTabsAndLabels();
}

void SendCoinsDialog::reject()
{
    clear();
}

void SendCoinsDialog::accept()
{
    clear();
    Q_EMIT notifyPaymentAccepted();
}

void SendCoinsDialog::deleteAddressBookEntry()
{
    //fixme: (FUT) Multiple entries.
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        GuldenSendCoinsEntry *entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry && !entry->isHidden())
        {
            entry->deleteAddressBookEntry();
            break;
        }
    }
}

void SendCoinsDialog::editAddressBookEntry()
{
    //fixme: (FUT) Multiple entries.
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        GuldenSendCoinsEntry *entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry && !entry->isHidden())
        {
            entry->editAddressBookEntry();
            break;
        }
    }
}

GuldenSendCoinsEntry *SendCoinsDialog::addEntry()
{
    GuldenSendCoinsEntry *entry = new GuldenSendCoinsEntry(platformStyle, this);
    entry->setModel(model);
    ui->entries->addWidget(entry);
    connect(entry, SIGNAL(removeEntry(GuldenSendCoinsEntry*)), this, SLOT(removeEntry(GuldenSendCoinsEntry*)));
    connect(entry, SIGNAL(valueChanged()), this, SLOT(coinControlUpdateLabels()));
    connect(entry, SIGNAL(subtractFeeFromAmountChanged()), this, SLOT(coinControlUpdateLabels()));

    connect(entry, SIGNAL(sendTabChanged()), this, SLOT(updateActionButtons()));

    // Focus the field, so that entry can start immediately
    entry->clear();
    entry->setFocus();
    ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
    qApp->processEvents();
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    if(bar)
        bar->setSliderPosition(bar->maximum());

    updateTabsAndLabels();
    updateActionButtonsForEntry(entry);
    return entry;
}

void SendCoinsDialog::updateTabsAndLabels()
{
    setupTabChain(0);
    coinControlUpdateLabels();
}

void SendCoinsDialog::removeEntry(GuldenSendCoinsEntry* entry)
{
    entry->hide();

    // If the last entry is about to be removed add an empty one
    if (ui->entries->count() == 1)
        addEntry();

    entry->deleteLater();

    updateTabsAndLabels();
}

QWidget *SendCoinsDialog::setupTabChain(QWidget *prev)
{
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        GuldenSendCoinsEntry *entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            prev = entry->setupTabChain(prev);
        }
    }
    QWidget::setTabOrder(prev, ui->sendButton);
    QWidget::setTabOrder(ui->sendButton, ui->clearButton);
    QWidget::setTabOrder(ui->clearButton, ui->addButton);
    return ui->addButton;
}

void SendCoinsDialog::setAddress(const QString &address)
{
    GuldenSendCoinsEntry *entry = 0;
    // Replace the first entry if it is still unused
    if(ui->entries->count() == 1)
    {
        GuldenSendCoinsEntry *first = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(0)->widget());
        if(first->isClear())
        {
            entry = first;
        }
    }
    if(!entry)
    {
        entry = addEntry();
    }

    entry->setAddress(address);
}

void SendCoinsDialog::pasteEntry(const SendCoinsRecipient &rv)
{
    if(!fNewRecipientAllowed)
        return;

    GuldenSendCoinsEntry *entry = 0;
    // Replace the first entry if it is still unused
    if(ui->entries->count() == 1)
    {
        GuldenSendCoinsEntry *first = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(0)->widget());
        if(first->isClear())
        {
            entry = first;
        }
    }
    if(!entry)
    {
        entry = addEntry();
    }

    entry->setValue(rv);
    updateTabsAndLabels();
}

bool SendCoinsDialog::handlePaymentRequest(const SendCoinsRecipient &rv)
{
    // Just paste the entry, all pre-checks
    // are done in paymentserver.cpp.
    pasteEntry(rv);
    return true;
}

void SendCoinsDialog::setBalance(WalletBalances balances, const CAmount& watchBalance, const CAmount& watchUnconfirmedBalance, const CAmount& watchImmatureBalance)
{
    (unused)watchBalance;
    (unused)watchUnconfirmedBalance;
    (unused)watchImmatureBalance;

    if(model && model->getOptionsModel())
    {
        ui->labelBalance->setText(GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), balances.availableExcludingLocked));
    }
}

void SendCoinsDialog::updateActionButtonsForEntry(GuldenSendCoinsEntry* entry)
{
      findChild<QPushButton*>("editButton")->setVisible(entry->ShouldShowEditButton());
      findChild<QPushButton*>("deleteButton")->setVisible(entry->ShouldShowDeleteButton());
      findChild<QPushButton*>("clearButton")->setVisible(entry->ShouldShowClearButton());
}

void SendCoinsDialog::updateActionButtons()
{
      QObject* sender = this->sender();
      GuldenSendCoinsEntry* entry = qobject_cast<GuldenSendCoinsEntry*>(sender);

      updateActionButtonsForEntry(entry);
}

void SendCoinsDialog::updateDisplayUnit()
{
    setBalance(model->getBalances(), model->getWatchBalance(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance());
    updateMinFeeLabel();
    updateSmartFeeLabel();
}

void SendCoinsDialog::processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg)
{
    QPair<QString, CClientUIInterface::MessageBoxFlags> msgParams;
    // Default to a warning message, override if error message is needed
    msgParams.second = CClientUIInterface::MSG_WARNING;

    // This comment is specific to SendCoinsDialog usage of WalletModel::SendCoinsReturn.
    // WalletModel::TransactionCommitFailed is used only in WalletModel::sendCoins()
    // all others are used only in WalletModel::prepareTransaction()
    switch(sendCoinsReturn.status)
    {
    case WalletModel::PoW2NotActive:
        msgParams.first = tr("PoW² is not yet active, please wait for activation and try again.");
        break;
    case WalletModel::InvalidAddress:
        msgParams.first = tr("The recipient address is not valid. Please recheck.");
        break;
    case WalletModel::InvalidAmount:
        msgParams.first = tr("The amount to pay must be larger than 0.");
        break;
    case WalletModel::AmountExceedsBalance:
        msgParams.first = tr("The amount exceeds your balance.\nIf you  have recently received funds you may need to wait for them to clear before spending them.");
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        msgParams.first = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgArg);
        break;
    case WalletModel::DuplicateAddress:
        msgParams.first = tr("Duplicate address found: addresses should only be used once each.");
        break;
    case WalletModel::TransactionCreationFailed:
        msgParams.first = tr("Transaction creation failed!");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::TransactionCommitFailed:
        msgParams.first = tr("The transaction was rejected with the following reason: %1").arg(sendCoinsReturn.reasonCommitFailed);
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::AbsurdFee:
        msgParams.first = tr("A fee higher than %1 is considered an absurdly high fee.").arg(GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), maxTxFee));
        break;
    case WalletModel::PaymentRequestExpired:
        msgParams.first = tr("Payment request expired.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::ForexFailed:
        msgParams.first = tr("Nocks request failed [%1]").arg(msgArg);
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    // included to prevent a compiler warning.
    case WalletModel::OK:
    default:
        return;
    }

    Q_EMIT message(tr("Send Coins"), msgParams.first, msgParams.second);
}

void SendCoinsDialog::minimizeFeeSection(bool fMinimize)
{
    ui->labelFeeMinimized->setVisible(fMinimize);
    ui->buttonChooseFee  ->setVisible(fMinimize);
    ui->buttonMinimizeFee->setVisible(!fMinimize);
    ui->frameFeeSelection->setVisible(!fMinimize);
    ui->horizontalLayoutSmartFee->setContentsMargins(0, (fMinimize ? 0 : 6), 0, 0);
    fFeeMinimized = fMinimize;
}

void SendCoinsDialog::on_buttonChooseFee_clicked()
{
    minimizeFeeSection(false);
}

void SendCoinsDialog::on_buttonMinimizeFee_clicked()
{
    updateFeeMinimizedLabel();
    minimizeFeeSection(true);
}

void SendCoinsDialog::setMinimumFee()
{
    ui->radioCustomPerKilobyte->setChecked(true);
    ui->customFee->setAmount(CWallet::GetRequiredFee(1000));
}

void SendCoinsDialog::updateFeeSectionControls()
{
    ui->sliderSmartFee          ->setEnabled(ui->radioSmartFee->isChecked());
    ui->labelSmartFee           ->setEnabled(ui->radioSmartFee->isChecked());
    ui->labelSmartFee2          ->setEnabled(ui->radioSmartFee->isChecked());
    ui->labelSmartFee3          ->setEnabled(ui->radioSmartFee->isChecked());
    ui->labelFeeEstimation      ->setEnabled(ui->radioSmartFee->isChecked());
    ui->labelSmartFeeNormal     ->setEnabled(ui->radioSmartFee->isChecked());
    ui->labelSmartFeeFast       ->setEnabled(ui->radioSmartFee->isChecked());
    ui->confirmationTargetLabel ->setEnabled(ui->radioSmartFee->isChecked());
    ui->checkBoxMinimumFee      ->setEnabled(ui->radioCustomFee->isChecked());
    ui->labelMinFeeWarning      ->setEnabled(ui->radioCustomFee->isChecked());
    ui->radioCustomPerKilobyte  ->setEnabled(ui->radioCustomFee->isChecked() && !ui->checkBoxMinimumFee->isChecked());
    ui->customFee               ->setEnabled(ui->radioCustomFee->isChecked() && !ui->checkBoxMinimumFee->isChecked());
}

void SendCoinsDialog::updateGlobalFeeVariables()
{
    if (ui->radioSmartFee->isChecked())
    {
        int nConfirmTarget = ui->sliderSmartFee->maximum() - ui->sliderSmartFee->value() + 2;
        payTxFee = CFeeRate(0);

        // show the estimated required time for confirmation
        ui->confirmationTargetLabel->setText(GUIUtil::formatDurationStr(nConfirmTarget * Params().GetConsensus().nPowTargetSpacing) + " / " + tr("%n block(s)", "", nConfirmTarget));
    }
    else
    {
        payTxFee = CFeeRate(ui->customFee->amount());
    }
}

void SendCoinsDialog::updateFeeMinimizedLabel()
{
    if(!model || !model->getOptionsModel())
        return;

    if (ui->radioSmartFee->isChecked())
        ui->labelFeeMinimized->setText(ui->labelSmartFee->text());
    else {
        ui->labelFeeMinimized->setText(GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), ui->customFee->amount()) +
            ((ui->radioCustomPerKilobyte->isChecked()) ? "/kB" : ""));
    }
}

void SendCoinsDialog::updateMinFeeLabel()
{
    if (model && model->getOptionsModel())
        ui->checkBoxMinimumFee->setText(tr("Pay only the required fee of %1").arg(
            GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), CWallet::GetRequiredFee(1000)) + "/kB")
        );
}

void SendCoinsDialog::updateSmartFeeLabel()
{
    if(!model || !model->getOptionsModel())
        return;

    int nBlocksToConfirm = ui->sliderSmartFee->maximum() - ui->sliderSmartFee->value() + 2;
    int estimateFoundAtBlocks = nBlocksToConfirm;
    CFeeRate feeRate = ::feeEstimator.estimateSmartFee(nBlocksToConfirm, &estimateFoundAtBlocks, ::mempool);
    if (feeRate <= CFeeRate(0)) // not enough data => minfee
    {
        ui->labelSmartFee->setText(GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(),
                                                                std::max(CWallet::fallbackFee.GetFeePerK(), CWallet::GetRequiredFee(1000))) + "/kB");
        ui->labelSmartFee2->show(); // (Smart fee not initialized yet. This usually takes a few blocks...)
        ui->labelFeeEstimation->setText("");
        ui->fallbackFeeWarningLabel->setVisible(true);
        int lightness = ui->fallbackFeeWarningLabel->palette().color(QPalette::WindowText).lightness();
        QColor warning_colour(255 - (lightness / 5), 176 - (lightness / 3), 48 - (lightness / 14));
        ui->fallbackFeeWarningLabel->setStyleSheet("QLabel { color: " + warning_colour.name() + "; }");
        ui->fallbackFeeWarningLabel->setIndent(QFontMetrics(ui->fallbackFeeWarningLabel->font()).width("x"));
    }
    else
    {
        ui->labelSmartFee->setText(GuldenUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(),
                                                                std::max(feeRate.GetFeePerK(), CWallet::GetRequiredFee(1000))) + "/kB");
        ui->labelSmartFee2->hide();
        ui->labelFeeEstimation->setText(tr("Estimated to begin confirmation within %n block(s).", "", estimateFoundAtBlocks));
        ui->fallbackFeeWarningLabel->setVisible(false);
    }

    updateFeeMinimizedLabel();
}

// Coin Control: copy label "Quantity" to clipboard
void SendCoinsDialog::coinControlClipboardQuantity()
{
    GUIUtil::setClipboard(ui->labelCoinControlQuantity->text());
}

// Coin Control: copy label "Amount" to clipboard
void SendCoinsDialog::coinControlClipboardAmount()
{
    GUIUtil::setClipboard(ui->labelCoinControlAmount->text().left(ui->labelCoinControlAmount->text().indexOf(" ")));
}

// Coin Control: copy label "Fee" to clipboard
void SendCoinsDialog::coinControlClipboardFee()
{
    GUIUtil::setClipboard(ui->labelCoinControlFee->text().left(ui->labelCoinControlFee->text().indexOf(" ")).replace(ASYMP_UTF8, ""));
}

// Coin Control: copy label "After fee" to clipboard
void SendCoinsDialog::coinControlClipboardAfterFee()
{
    GUIUtil::setClipboard(ui->labelCoinControlAfterFee->text().left(ui->labelCoinControlAfterFee->text().indexOf(" ")).replace(ASYMP_UTF8, ""));
}

// Coin Control: copy label "Bytes" to clipboard
void SendCoinsDialog::coinControlClipboardBytes()
{
    GUIUtil::setClipboard(ui->labelCoinControlBytes->text().replace(ASYMP_UTF8, ""));
}

// Coin Control: copy label "Dust" to clipboard
void SendCoinsDialog::coinControlClipboardLowOutput()
{
    GUIUtil::setClipboard(ui->labelCoinControlLowOutput->text());
}

// Coin Control: copy label "Change" to clipboard
void SendCoinsDialog::coinControlClipboardChange()
{
    GUIUtil::setClipboard(ui->labelCoinControlChange->text().left(ui->labelCoinControlChange->text().indexOf(" ")).replace(ASYMP_UTF8, ""));
}

// Coin Control: settings menu - coin control enabled/disabled by user
void SendCoinsDialog::coinControlFeatureChanged(bool checked)
{
    ui->frameCoinControl->setVisible(checked);

    if (!checked && model) // coin control features disabled
        CoinControlDialog::coinControl->SetNull();

    // make sure we set back the confirmation target
    updateGlobalFeeVariables();
    coinControlUpdateLabels();
}

// Coin Control: button inputs -> show actual coin control dialog
void SendCoinsDialog::coinControlButtonClicked()
{
    CoinControlDialog dlg(platformStyle);
    dlg.setModel(model);
    dlg.exec();
    coinControlUpdateLabels();
}

// Coin Control: checkbox custom change address
void SendCoinsDialog::coinControlChangeChecked(int state)
{
    if (state == Qt::Unchecked)
    {
        CoinControlDialog::coinControl->destChange = CNoDestination();
        ui->labelCoinControlChangeLabel->clear();
    }
    else
        // use this to re-validate an already entered address
        coinControlChangeEdited(ui->lineEditCoinControlChange->text());

    ui->lineEditCoinControlChange->setEnabled((state == Qt::Checked));
}

// Coin Control: custom change address changed
void SendCoinsDialog::coinControlChangeEdited(const QString& text)
{
    if (model && model->getAddressTableModel())
    {
        // Default to no change address until verified
        CoinControlDialog::coinControl->destChange = CNoDestination();
        ui->labelCoinControlChangeLabel->setStyleSheet("QLabel{color:red;}");

        CNativeAddress addr = CNativeAddress(text.toStdString());

        if (text.isEmpty()) // Nothing entered
        {
            ui->labelCoinControlChangeLabel->setText("");
        }
        else if (!addr.IsValid()) // Invalid address
        {
            ui->labelCoinControlChangeLabel->setText(tr("Warning: Invalid Gulden address"));
        }
        else // Valid address
        {
            CKeyID keyid;
            addr.GetKeyID(keyid);
            if (!model->havePrivKey(keyid)) // Unknown change address
            {
                ui->labelCoinControlChangeLabel->setText(tr("Warning: Unknown change address"));

                // confirmation dialog
                QMessageBox::StandardButton btnRetVal = QMessageBox::question(this, tr("Confirm custom change address"), tr("The address you selected for change is not part of this wallet. Any or all funds in your wallet may be sent to this address. Are you sure?"),
                    QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

                if(btnRetVal == QMessageBox::Yes)
                    CoinControlDialog::coinControl->destChange = addr.Get();
                else
                {
                    ui->lineEditCoinControlChange->setText("");
                    ui->labelCoinControlChangeLabel->setStyleSheet("QLabel{color:black;}");
                    ui->labelCoinControlChangeLabel->setText("");
                }
            }
            else // Known change address
            {
                ui->labelCoinControlChangeLabel->setStyleSheet("QLabel{color:black;}");

                // Query label
                QString associatedLabel = model->getAddressTableModel()->labelForAddress(text);
                if (!associatedLabel.isEmpty())
                    ui->labelCoinControlChangeLabel->setText(associatedLabel);
                else
                    ui->labelCoinControlChangeLabel->setText(tr("(no label)"));

                CoinControlDialog::coinControl->destChange = addr.Get();
            }
        }
    }
}

// Coin Control: update labels
void SendCoinsDialog::coinControlUpdateLabels()
{
    if (!model || !model->getOptionsModel())
        return;

    // set pay amounts
    CoinControlDialog::payAmounts.clear();
    CoinControlDialog::fSubtractFeeFromAmount = false;
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        GuldenSendCoinsEntry *entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry && !entry->isHidden())
        {
            SendCoinsRecipient rcp = entry->getValue(false);
            CoinControlDialog::payAmounts.append(rcp.amount);
            if (rcp.fSubtractFeeFromAmount)
                CoinControlDialog::fSubtractFeeFromAmount = true;
        }
    }

    if (CoinControlDialog::coinControl->HasSelected())
    {
        // actual coin control calculation
        CoinControlDialog::updateLabels(model, this);

        // show coin control stats
        ui->labelCoinControlAutomaticallySelected->hide();
        ui->widgetCoinControl->show();
    }
    else
    {
        // hide coin control stats
        ui->labelCoinControlAutomaticallySelected->show();
        ui->widgetCoinControl->hide();
        ui->labelCoinControlInsuffFunds->hide();
    }
}

void SendCoinsDialog::update()
{
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        GuldenSendCoinsEntry *entry = qobject_cast<GuldenSendCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            entry->update();
        }
    }
}

SendConfirmationDialog::SendConfirmationDialog(const QString &title, const QString &text, int _secDelay,
    QWidget *parent) :
    QMessageBox(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::Cancel, parent), secDelay(_secDelay)
{
    setDefaultButton(QMessageBox::Cancel);
    yesButton = button(QMessageBox::Yes);
    updateYesButton();
    connect(&countDownTimer, SIGNAL(timeout()), this, SLOT(countDown()));
}

int SendConfirmationDialog::exec()
{
    updateYesButton();
    countDownTimer.start(1000);
    return QMessageBox::exec();
}

void SendConfirmationDialog::countDown()
{
    secDelay--;
    updateYesButton();

    if(secDelay <= 0)
    {
        countDownTimer.stop();
    }
}

void SendConfirmationDialog::updateYesButton()
{
    if(secDelay > 0)
    {
        yesButton->setEnabled(false);
        yesButton->setText(tr("Yes") + " (" + QString::number(secDelay) + ")");
    }
    else
    {
        yesButton->setEnabled(true);
        yesButton->setText(tr("Yes"));
    }
}
