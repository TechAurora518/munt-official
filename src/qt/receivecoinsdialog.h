// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_GULDENRECEIVECOINSDIALOG_H
#define GULDEN_QT_GULDENRECEIVECOINSDIALOG_H

#include "guiutil.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

#include <QDialog>
#include <QHeaderView>
#include <QItemSelection>
#include <QKeyEvent>
#include <QMenu>


class OptionsModel;
class QStyle;
class WalletModel;
class CReserveKeyOrScript;

class CAccount;

namespace Ui {
    class ReceiveCoinsDialog;
}

class ReceiveCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiveCoinsDialog(const QStyle *platformStyle, QWidget *parent = 0);
    ~ReceiveCoinsDialog();

    void setModel(WalletModel *model);
    void updateQRCode(const QString& sAddress);
    void setActiveAccount(CAccount* account);
    void setShowCopyQRAsImageButton(bool showCopyQRAsImagebutton_);
    static bool showCopyQRAsImagebutton;

public Q_SLOTS:
    void updateAddress(const QString& address);
    void showBuyGuldenDialog();
    void gotoRequestPaymentPage();
    void cancelRequestPayment();
    void gotoReceievePage();
    void activeAccountChanged(CAccount* activeAccount);

protected:

private:
    Ui::ReceiveCoinsDialog* ui;
    WalletModel* model;
    const QStyle* platformStyle;
    QString accountAddress;
    CReserveKeyOrScript* buyReceiveAddress;
    CAccount* currentAccount;

private Q_SLOTS:
  void copyAddressToClipboard();
  void saveQRAsImage();
  void generateRequest();
};

#endif // GULDEN_QT_GULDENRECEIVECOINSDIALOG_H
