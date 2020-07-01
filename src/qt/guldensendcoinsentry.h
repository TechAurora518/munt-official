// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_SENDCOINSENTRY_H
#define GULDEN_QT_SENDCOINSENTRY_H

#if defined(HAVE_CONFIG_H)
#include "config/build-config.h"
#endif

#ifdef ENABLE_WALLET
#include "walletmodel.h"
#endif

#include <QStackedWidget>

class WalletModel;
class QStyle;
class QSortFilterProxyModel;
class NocksRequest;

namespace Ui {
    class GuldenSendCoinsEntry;
}

/**
 * A single entry in the dialog for sending transactions.
 */
class GuldenSendCoinsEntry : public QFrame
{
    Q_OBJECT

public:
    explicit GuldenSendCoinsEntry(const QStyle *platformStyle, QWidget *parent = 0);
    ~GuldenSendCoinsEntry();

    void setModel(WalletModel *model);
    bool validate();
    SendCoinsRecipient::PaymentType getPaymentType(const QString& fromAddress);
    SendCoinsRecipient getValue(bool showWarningDialogs=true);

    /** Return whether the entry is still empty and unedited */
    bool isClear();

    void update();

    void setValue(const SendCoinsRecipient &value);
    void setAmount(const CAmount amount);
    void setAddress(const QString &address);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases
     *  (issue https://bugreports.qt-project.org/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void setFocus();

    bool ShouldShowEditButton() const;
    bool ShouldShowClearButton() const;
    bool ShouldShowDeleteButton() const;;

    void deleteAddressBookEntry();
    void editAddressBookEntry();

public Q_SLOTS:
    void clear();
    void addressChanged();
    void receivingAccountNameChanged();
    void tabChanged();
    void addressBookSelectionChanged();
    void myAccountsSelectionChanged();

Q_SIGNALS:
    void removeEntry(GuldenSendCoinsEntry *entry);
    void subtractFeeFromAmountChanged();
    void sendTabChanged();
    void valueChanged();

private Q_SLOTS:
    void deleteClicked();
    void updateDisplayUnit();
    void searchChangedAddressBook(const QString& searchString);
    void searchChangedMyAccounts(const QString& searchString);
    void payAmountChanged();
    void nocksQuoteProcessed();
    void nocksTimeout();
    void sendAllClicked();

private:

    SendCoinsRecipient recipient;
    Ui::GuldenSendCoinsEntry *ui;
    WalletModel *model;
    const QStyle *platformStyle;

    QSortFilterProxyModel* proxyModelRecipients;
    QSortFilterProxyModel* proxyModelAddresses;

    bool updateLabel(const QString &address);

    void payInfoUpdateRequired();
    void setPayInfo(const QString& msg, bool attention=false);
    void clearPayInfo();

    void cancelNocksQuote();

    NocksRequest* nocksQuote;
    QTimer* nocksTimer;
};

#endif // GULDEN_QT_SENDCOINSENTRY_H
