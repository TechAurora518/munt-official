// Copyright (c) 2015-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_TICKER_H
#define GULDEN_TICKER_H

#include <string>
#include <map>

#include <QObject>
#include <QSslError>
#include <QList>
#include <QAbstractTableModel>
#include "amount.h"


class OptionsModel;
class QNetworkAccessManager;
class QNetworkReply;
class CurrencyTicker;
struct WalletBalances;


class CurrencyTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum MapRoles {
        KeyRole = Qt::UserRole + 1,
        ValueRole
    };

    explicit CurrencyTableModel(QObject *parent, CurrencyTicker* ticker);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    inline void setMap(std::map<std::string, std::string>* currenciesMap) { m_currenciesMap = currenciesMap; }

    void setBalance(CAmount balanceNLG);

public Q_SLOTS:
    void balanceChanged(const WalletBalances& balances, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

private:
    CurrencyTicker* m_ticker;
    CAmount m_balanceNLG;
    std::map<std::string, std::string>* m_currenciesMap;
};


class CurrencyTicker : public QObject
{
    Q_OBJECT

public:
    // parent should be QApplication object
    CurrencyTicker(QObject* parent);
    virtual ~CurrencyTicker();

    CAmount convertGuldenToForex(CAmount guldenAmount, std::string forexCurrencyCode);
    CAmount convertForexToGulden(CAmount forexAmount, std::string forexCurrencyCode);

    CurrencyTableModel* GetCurrencyTableModel();

Q_SIGNALS:
    void exchangeRatesUpdated();
    void exchangeRatesUpdatedLongPoll();

public Q_SLOTS:
    void pollTicker();

private Q_SLOTS:
    void netRequestFinished(QNetworkReply*);
    void reportSslErrors(QNetworkReply*, const QList<QSslError> &);

protected:
private:
    QNetworkAccessManager* netManager;
    OptionsModel *optionsModel;
    std::map<std::string, std::string> m_ExchangeRates;
    unsigned int count;
};

#endif
