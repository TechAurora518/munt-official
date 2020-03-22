// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "uritests.h"

#include "guiutil.h"
#include "walletmodel.h"

#include <QUrl>

void URITests::uriTests()
{
    SendCoinsRecipient rv;
    QUrl uri;
    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?req-dontexist="));
    QVERIFY(!GUIUtil::parseGuldenURI(uri, &rv));

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?dontexist="));
    QVERIFY(GUIUtil::parseGuldenURI(uri, &rv));
    QVERIFY(rv.address == QString("G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?label=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseGuldenURI(uri, &rv));
    QVERIFY(rv.address == QString("G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString("Wikipedia Example Address"));
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=0.001"));
    QVERIFY(GUIUtil::parseGuldenURI(uri, &rv));
    QVERIFY(rv.address == QString("G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100000);

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=1.001"));
    QVERIFY(GUIUtil::parseGuldenURI(uri, &rv));
    QVERIFY(rv.address == QString("G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100100000);

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=100&label=Wikipedia Example"));
    QVERIFY(GUIUtil::parseGuldenURI(uri, &rv));
    QVERIFY(rv.address == QString("G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.amount == 10000000000LL);
    QVERIFY(rv.label == QString("Wikipedia Example"));

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseGuldenURI(uri, &rv));
    QVERIFY(rv.address == QString("G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());

    QVERIFY(GUIUtil::parseGuldenURI("gulden://G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?message=Wikipedia Example Address", &rv));
    QVERIFY(rv.address == QString("G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?req-message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseGuldenURI(uri, &rv));

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=1,000&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseGuldenURI(uri, &rv));

    uri.setUrl(QString("gulden:G75tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=1,000.0&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseGuldenURI(uri, &rv));
}
