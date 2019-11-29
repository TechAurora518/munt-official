// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "nocksrequest.h"
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

#include "walletmodel.h"


#include <ostream>
#include <iomanip>
#include <sstream>

#include <openssl/x509_vfy.h>

static QString nocksHost()
{
    if (IsArgSet("-testnet"))
        return QString("sandbox.nocks.com");
    else
        return QString("www.nocks.com");
}

NocksRequest::NocksRequest( QObject* parent)
: optionsModel( nullptr )
, m_recipient( nullptr )
, networkReply( nullptr )
{
    netManager = new QNetworkAccessManager( this );
    netManager->setObjectName("nocks_request_manager");

    connect( netManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( netRequestFinished( QNetworkReply* ) ) );
    connect( netManager, SIGNAL( sslErrors( QNetworkReply*, const QList<QSslError>& ) ), this, SLOT( reportSslErrors( QNetworkReply*, const QList<QSslError>& ) ) );
}

NocksRequest::~NocksRequest()
{
    cancel();
    delete netManager;
}

void NocksRequest::startRequest(SendCoinsRecipient* recipient, RequestType type, QString from, QString to, QString amount, QString description)
{
    assert(networkReply == nullptr);

    m_recipient = recipient;
    requestType = type;

    QNetworkRequest netRequest;
    QString httpPostParamaters;

    if (requestType == RequestType::Quotation)
    {
        httpPostParamaters = QString("{\"pair\": \"%1_%2\", \"amount\": \"%3\", \"fee\": \"yes\", \"amountType\": \"withdrawal\"}").arg(from, to, amount);
        netRequest.setUrl( QString("https://%1/api/price").arg(nocksHost()));
    }
    else
    {
        if (m_recipient)
        {
            originalAddress = m_recipient->address.toStdString();
        }

        QString forexCurrencyType;
        if (recipient->paymentType == SendCoinsRecipient::PaymentType::BitcoinPayment)
            forexCurrencyType = "BTC";
        else if (recipient->paymentType == SendCoinsRecipient::PaymentType::IBANPayment)
            forexCurrencyType = "EUR";

        //Stop infinite recursion.
        if (m_recipient)
        {
            m_recipient->forexPaymentType = m_recipient->paymentType;
            m_recipient->forexAmount = m_recipient->amount;
            m_recipient->paymentType = SendCoinsRecipient::PaymentType::NormalPayment;
            description = m_recipient->forexDescription;
        }
               
        //fixme: (FUT) (SEPA)
        QString httpExtraParams = "";
        /*
        if (forexExtraName != null && !forexExtraName.isEmpty())
            httpExtraParams = httpExtraParams + String.format(", \"name\": \"%s\"", forexExtraName);*/
        if (!description.isEmpty())
            httpExtraParams = httpExtraParams + ", \"text\": \"" + description + "\"";
        /*if (forexExtraRemmitance2 != null && !forexExtraRemmitance2.isEmpty())
            httpExtraParams = httpExtraParams + String.format(", \"reference\": \"%s\"", forexExtraRemmitance2);*/

        QString forexAmount = GuldenUnits::format(GuldenUnits::NLG, recipient->amount, false, GuldenUnits::separatorNever);

        httpPostParamaters = QString("{\"pair\": \"NLG_%1\", \"amount\": \"%2\", \"withdrawal\": \"%3\"%4}").arg(forexCurrencyType, forexAmount, recipient->address, httpExtraParams);
        netRequest.setUrl( QString("https://%1/api/transaction").arg(nocksHost()));
    }

    netRequest.setRawHeader( "User-Agent", QByteArray(UserAgent().c_str()));
    netRequest.setRawHeader( "Accept", "application/json" );
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QSslConfiguration config( QSslConfiguration::defaultConfiguration() );
    netRequest.setSslConfiguration( config );

    QByteArray data = httpPostParamaters.toStdString().c_str();

    networkReply = netManager->post( netRequest, data );
}

void NocksRequest::cancel()
{
    netManager->disconnect(this);

    if (networkReply)
    {
        networkReply->abort();
        networkReply->deleteLater();
        networkReply = nullptr;
    }
}

void NocksRequest::setOptionsModel( OptionsModel* optionsModel_ )
{
    /*optionsModel = optionsModel;

    if ( optionsModel )
    {
        QNetworkProxy proxy;

        // Query active SOCKS5 proxy
        if ( optionsModel->getProxySettings( proxy ) )
        {
            netManager->setProxy( proxy );
            qDebug() << "PaymentServer::initNetManager: Using SOCKS5 proxy" << proxy.hostName() << ":" << proxy.port();
        }
        else
            qDebug() << "PaymentServer::initNetManager: No active proxy server found.";
    }*/
}

void NocksRequest::netRequestFinished( QNetworkReply* reply )
{
    assert(networkReply == reply);

    if ( reply->error() != QNetworkReply::NetworkError::NoError )
    {
        //fixme: (FUT) Better error code
        if (m_recipient)
            m_recipient->forexFailCode = "Nocks is temporarily unreachable, please try again later.";
        Q_EMIT requestProcessed();
        return;
    }
    else
    {
        int statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

        if ( statusCode < 200 && statusCode > 202 )
        {
            //fixme: (FUT)  Better error code
            if (m_recipient)
            {
                m_recipient->forexFailCode = "Nocks is temporarily unreachable, please try again later.";
                Q_EMIT requestProcessed();
            }
            return;
        }
        else
        {
            QByteArray jsonReply = reply->readAll();
            QString temp = QString::fromStdString( jsonReply.data() );
            QJsonDocument jsonDoc = QJsonDocument::fromJson( jsonReply );
            QJsonValue successValue = jsonDoc.object().value( "success" );
            QJsonValue errorValue = jsonDoc.object().value( "error" );

            if (errorValue != QJsonValue::Undefined)
            {
                if (m_recipient)
                {
                    m_recipient->forexFailCode = errorValue.isString() ?
                                errorValue.toString().toStdString()
                              : "Could not process your request, please try again later.";
                    Q_EMIT requestProcessed();
                }
                return;
            }
            else if (successValue == QJsonValue::Undefined)
            {
                //fixme: (FUT)  Better error code
                if (m_recipient)
                {
                    m_recipient->forexFailCode = "Nocks is temporarily unreachable, please try again later.";
                    Q_EMIT requestProcessed();
                }
                return;
            }
            else
            {
                if (requestType == RequestType::Quotation)
                {
                    GuldenUnits::parse(GuldenUnits::NLG, successValue.toObject().value("amount").toString(), &nativeAmount);
                }
                else
                {
                    QString depositAmount = successValue.toObject().value("depositAmount").toString();
                    QString depositAddress = successValue.toObject().value("deposit").toString();
                    QString expirationTime = successValue.toObject().value("expirationTimestamp").toString();
                    QString withdrawalAmount = successValue.toObject().value("withdrawalAmount").toString();
                    QString withdrawalAddress = successValue.toObject().value("withdrawalOriginal").toString();

                    //fixme: (FUT)  Should check amount adds up as well, but can't because of fee... - Manually subtract fee and verify it all adds up?
                    if (withdrawalAddress.toStdString() != originalAddress )
                    {
                        m_recipient->forexFailCode = "Withdrawal address modified, please contact a developer for assistance.";
                        Q_EMIT requestProcessed();
                        return;
                    }
                    m_recipient->paymentType = SendCoinsRecipient::PaymentType::NormalPayment;
                    m_recipient->forexAddress = QString::fromStdString(originalAddress);
                    m_recipient->address = depositAddress;
                    GuldenUnits::parse(GuldenUnits::NLG, depositAmount, &m_recipient->amount);
                    m_recipient->expiry = expirationTime.toLong() - 120;
                    m_recipient->forexFailCode = "";
                }

                Q_EMIT requestProcessed();
                return;
            }
        }
    }
}

void NocksRequest::reportSslErrors( [[maybe_unused]] QNetworkReply* reply, [[maybe_unused]] const QList<QSslError>& errorList )
{
    if (m_recipient)
        m_recipient->forexFailCode = "Nocks is temporarily unreachable, please try again later.";
    Q_EMIT requestProcessed();
}





