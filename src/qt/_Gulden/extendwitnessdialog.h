// Copyright (c) 2016-2019 The Gulden developers
// Authored by: Willem de Jonge (willem@isnapp.nl)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_EXTENDWITNESSDIALOG_H
#define GULDEN_QT_EXTENDWITNESSDIALOG_H

#include "guiutil.h"

#include <QFrame>
#include <QHeaderView>
#include <QItemSelection>
#include <QKeyEvent>
#include <QMenu>
#include <QPoint>
#include <QVariant>

class OptionsModel;
class QStyle;
class WalletModel;

namespace Ui {
    class ExtendWitnessDialog;
}

class ExtendWitnessDialog : public QFrame
{
    Q_OBJECT

public:
    explicit ExtendWitnessDialog(CAmount lockedAmount_, int durationRemaining, int64_t minimumWeight, WalletModel* walletModel_, const QStyle *platformStyle, QWidget *parent = 0);
    ~ExtendWitnessDialog();

Q_SIGNALS:
    void dismiss(QWidget*);

protected:

private:
    Ui::ExtendWitnessDialog *ui;
    const QStyle *platformStyle;
    WalletModel* walletModel;
    CAmount lockedAmount;

private Q_SLOTS:
    void cancelClicked();
    void extendClicked();
    void amountFieldChanged();
};

#endif // GULDEN_QT_EXTENDWITNESSDIALOG_H
