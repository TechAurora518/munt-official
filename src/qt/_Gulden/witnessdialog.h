// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_WITNESSDIALOG_H
#define GULDEN_QT_WITNESSDIALOG_H

#include "guiutil.h"
#include <transactionfilterproxy.h>


#include <QFrame>

class QMenu;
class PlatformStyle;
class QwtPlotCurve;
class ClientModel;
class OptionsModel;
class WalletModel;
class CAccount;

namespace Ui {
    class WitnessDialog;
}



class WitnessDialog : public QFrame
{
    Q_OBJECT

public:
    explicit WitnessDialog(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~WitnessDialog();

    void setClientModel(ClientModel *clientModel);
    void setModel(WalletModel *model);

Q_SIGNALS:
    void requestEmptyWitness();
    void requestFundWitness(CAccount* funderAccount);
    void requestRenewWitness(CAccount* funderAccount);

public Q_SLOTS:
    void updateUnit(int nNewUnit_);
    void plotGraphForAccount(CAccount* account);
    void update();
    void unitButtonClicked();
    void viewWitnessInfoClicked();
    void emptyWitnessClicked();
    void fundWitnessClicked();
    void renewWitnessClicked();
protected:

private:
    Ui::WitnessDialog *ui;
    const PlatformStyle *platformStyle;
    ClientModel *clientModel;
    WalletModel *model;

    QwtPlotCurve* expectedEarningsCurve = nullptr;
    QwtPlotCurve* currentEarningsCurveShadow = nullptr;
    QwtPlotCurve* currentEarningsCurveForecastShadow = nullptr;
    QwtPlotCurve* currentEarningsCurve = nullptr;
    QwtPlotCurve* currentEarningsCurveForecast = nullptr;

    QMenu* unitSelectionMenu;

    std::unique_ptr<TransactionFilterProxy> filter;

private Q_SLOTS:

};

#endif // GULDEN_QT_WITNESSDIALOG_H
