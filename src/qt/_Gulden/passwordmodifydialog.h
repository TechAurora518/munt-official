// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_PASSWORDMODIFYDIALOG_H
#define GULDEN_QT_PASSWORDMODIFYDIALOG_H

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
    class PasswordModifyDialog;
}

class PasswordModifyDialog : public QFrame
{
    Q_OBJECT

public:
    explicit PasswordModifyDialog(const QStyle *platformStyle, QWidget *parent = 0);
    ~PasswordModifyDialog();

Q_SIGNALS:
    void dismiss();

public Q_SLOTS:

protected:

private:
    Ui::PasswordModifyDialog *ui;
    const QStyle *platformStyle;

private Q_SLOTS:
    void setPassword();
    void oldPasswordChanged();
    void newPasswordChanged();
    void newPasswordRepeatChanged();
};

#endif // GULDEN_QT_PASSWORDMODIFYDIALOG_H
