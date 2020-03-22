// Copyright (c) 2015-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "clientversion.h"
#include "ticker.h"
#include "utilmoneystr.h"
#include "arith_uint256.h"
#include "units.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QVariant>
#include <QModelIndex>
#include "wallet/wallet.h"


#include <ostream>
#include <iomanip>
#include <sstream>

#include <openssl/x509_vfy.h>

CurrencyTableModel::CurrencyTableModel(QObject *parent, CurrencyTicker* ticker)
: QAbstractTableModel( parent )
, m_ticker( ticker )
, m_currenciesMap( NULL )
{
}

int CurrencyTableModel::rowCount(const QModelIndex& parent) const
{
    if (m_currenciesMap)
        return m_currenciesMap->size();

    return 0;
}

int CurrencyTableModel::columnCount(const QModelIndex & parent) const
{
    return 3;
}

QVariant CurrencyTableModel::data(const QModelIndex& index, int role) const
{
    if (!m_currenciesMap)
        return QVariant();

    if (index.row() < 0 || index.row() >= (int)m_currenciesMap->size() || role != Qt::DisplayRole)
    {
        return QVariant();
    }

    auto iter = m_currenciesMap->begin();
    std::advance(iter, index.row());

    if (index.column() == 0)
    {
        return QString::fromStdString(iter->first);
    }
    if (index.column() == 1)
    {
        return QString("rate<br/>balance");
    }
    if (index.column() == 2)
    {
        CAmount temp;
        ParseMoney(iter->second,temp);
        //fixme: (FUT) Truncates - we should instead round here...
        QString rate = GuldenUnits::format(GuldenUnits::NLG, temp, false, GuldenUnits::separatorAlways, 4);
        QString balance = GuldenUnits::format(GuldenUnits::NLG, m_ticker->convertGuldenToForex(m_balanceNLG, iter->first), false, GuldenUnits::separatorAlways, 2);
        return rate + QString("<br/>") + balance;
    }

    return QVariant();
}


void CurrencyTableModel::setBalance(CAmount balanceNLG)
{
    m_balanceNLG = balanceNLG;
}

void CurrencyTableModel::balanceChanged(const WalletBalances& balances, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    setBalance(balances.availableIncludingLocked + balances.unconfirmedIncludingLocked + balances.immatureIncludingLocked);
    //fixme: (FUT) Only emit if data actually changes.
    Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(rowCount()-1, columnCount()-1, QModelIndex()));
}

CurrencyTicker::CurrencyTicker( QObject* parent )
: count(0)
{
    netManager = new QNetworkAccessManager( this );
    netManager->setObjectName("currency_ticker_net_manager");

    connect( netManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( netRequestFinished( QNetworkReply* ) ) );
    connect( netManager, SIGNAL( sslErrors( QNetworkReply*, const QList<QSslError>& ) ), this, SLOT( reportSslErrors( QNetworkReply*, const QList<QSslError>& ) ) );
}

CurrencyTicker::~CurrencyTicker()
{
    disconnect( netManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( netRequestFinished( QNetworkReply* ) ) );
    disconnect( netManager, SIGNAL( sslErrors( QNetworkReply*, const QList<QSslError>& ) ), this, SLOT( reportSslErrors( QNetworkReply*, const QList<QSslError>& ) ) );
}

CAmount CurrencyTicker::convertGuldenToForex(CAmount guldenAmount, std::string forexCurrencyCode)
{
    if (m_ExchangeRates.find(forexCurrencyCode) != m_ExchangeRates.end())
    {
        CAmount exchangeRate;
        if ( ParseMoney(m_ExchangeRates[forexCurrencyCode], exchangeRate) )
        {
            arith_uint256 forexAmountBN = guldenAmount;
            forexAmountBN *= exchangeRate;
            forexAmountBN /= COIN;
            return forexAmountBN.GetLow64();
        }
    }
    return CAmount(0);
}


CAmount CurrencyTicker::convertForexToGulden(CAmount forexAmount, std::string forexCurrencyCode)
{
    if (m_ExchangeRates.find(forexCurrencyCode) != m_ExchangeRates.end())
    {
        CAmount exchangeRate;
        if ( ParseMoney(m_ExchangeRates[forexCurrencyCode], exchangeRate) )
        {
            arith_uint256 forexAmountBN = forexAmount;
            forexAmountBN *= COIN;
            if (exchangeRate != 0)
            {
                forexAmountBN /= exchangeRate;
                return forexAmountBN.GetLow64();
            }
        }
    }
    return CAmount(0);
}

CurrencyTableModel* CurrencyTicker::GetCurrencyTableModel()
{
    CurrencyTableModel* model = new CurrencyTableModel(this, this);
    model->setObjectName("ticker_currency_table_model");
    model->setMap(&m_ExchangeRates);
    model->setBalance(CAmount(0));
    return model;
}

void CurrencyTicker::pollTicker()
{
    QNetworkRequest netRequest;
    netRequest.setUrl( QString::fromStdString( "https://api.gulden.com/api/v1/ticker" ) );
    netRequest.setRawHeader( "User-Agent", QByteArray(UserAgent().c_str()));
    netRequest.setRawHeader( "Accept", "application/json" );

    QSslConfiguration config( QSslConfiguration::defaultConfiguration() );
    netRequest.setSslConfiguration( config );

    netManager->get( netRequest );
}

void CurrencyTicker::netRequestFinished( QNetworkReply* reply )
{
    reply->deleteLater();
    bool signalUpdates = false;

    if ( reply->error() != QNetworkReply::NetworkError::NoError )
    {
        //fixme: (FUT) Error handling code here.
        //Note - it is possible the ticker has temporary outages etc. and these are not a major issue
        //We update every ~10s but if we miss a few updates it has no ill-effects
        //So if we do anything here, it should only be after multiple failiures...
    }
    else
    {
        int statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

        if ( statusCode != 200 )
        {
            //fixme: (FUT) Error handling code here.
            //Note - it is possible the ticker has temporary outages etc. and these are not a major issue
            //We update every ~10s but if we miss a few updates it has no ill-effects
            //So if we do anything here, it should only be after multiple failiures...
        }
        else
        {
            QByteArray jsonReply = reply->readAll();
            QString temp = QString::fromStdString( jsonReply.data() );
            QJsonDocument jsonDoc = QJsonDocument::fromJson( jsonReply );
            QJsonArray jsonArray = jsonDoc.object().value( "data" ).toArray();

            for ( auto jsonVal : jsonArray )
            {
                QJsonObject jsonObject = jsonVal.toObject();
                std::string currencyName = jsonObject.value( "name" ).toString().toStdString();
                std::string currencyCode = jsonObject.value( "code" ).toString().toStdString();
                //NB! Possible precision loss here - but in this case it is probably acceptible.
                std::string currencyRate;
                std::ostringstream bufstream;
                bufstream << std::fixed << std::setprecision(8) << jsonObject.value( "rate" ).toDouble();
                currencyRate = bufstream.str();

                m_ExchangeRates[currencyCode] = currencyRate;
                signalUpdates = true;
            }
        }
    }

    if ( signalUpdates )
    {
        Q_EMIT exchangeRatesUpdated();
        if(++count % 20 == 0)
        {
            Q_EMIT exchangeRatesUpdatedLongPoll();
        }
    }

    // Call again every ~10 seconds
    QTimer::singleShot( 10000, this, SLOT(pollTicker()) );
}

void CurrencyTicker::reportSslErrors( QNetworkReply* reply, const QList<QSslError>& errorList )
{
    //fixme: (FUT) In future (I guess) we should signal in UI somehow that ticker is unavailable - need to decide how to do this in a user friendly way.
    //Note - it is possible the ticker has temporary outages - is switching hosts or whatever other minor thing 
    //We update every ~10s but if we miss a few updates it has no ill-effects
    //So if we do anything here, it should only be after multiple failiures...
}





