// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "passwordmodifydialog.h"
#include <qt/forms/ui_passwordmodifydialog.h>

#include "wallet/wallet.h"
#include "GuldenGUI.h"



PasswordModifyDialog::PasswordModifyDialog(const QStyle *_platformStyle, QWidget *parent)
: QFrame( parent )
, ui( new Ui::PasswordModifyDialog )
, platformStyle( _platformStyle )
{
    ui->setupUi(this);

    ui->buttonCancel->setCursor(Qt::PointingHandCursor);
    ui->buttonSave->setCursor(Qt::PointingHandCursor);

    connect(ui->buttonCancel, SIGNAL(clicked()), this, SIGNAL(dismiss()));
    connect(ui->buttonSave, SIGNAL(clicked()), this, SLOT(setPassword()));
    connect(ui->lineEditOldPassword, SIGNAL(textEdited(QString)), this, SLOT(oldPasswordChanged()));
    connect(ui->lineEditNewPassword, SIGNAL(textEdited(QString)), this, SLOT(newPasswordChanged()));
    connect(ui->lineEditNewPasswordRepeat, SIGNAL(textEdited(QString)), this, SLOT(newPasswordRepeatChanged()));

    if (pactiveWallet->IsCrypted())
    {
        ui->lineEditOldPassword->setVisible(true);
    }
    else
    {
        ui->lineEditOldPassword->setVisible(false);
    }
}


PasswordModifyDialog::~PasswordModifyDialog()
{
    burnLineEditMemory(ui->lineEditOldPassword);
       burnLineEditMemory(ui->lineEditNewPassword);
    burnLineEditMemory(ui->lineEditNewPasswordRepeat);
    delete ui;
}

void PasswordModifyDialog::oldPasswordChanged()
{
    setValid(ui->lineEditOldPassword, true);
}

void PasswordModifyDialog::newPasswordChanged()
{
    setValid(ui->lineEditNewPassword, true);
}

void PasswordModifyDialog::newPasswordRepeatChanged()
{
    setValid(ui->lineEditNewPasswordRepeat, true);
}

void PasswordModifyDialog::setPassword()
{
    if (pactiveWallet->IsCrypted())
    {
        if (ui->lineEditOldPassword->text().isEmpty())
        {
            setValid(ui->lineEditOldPassword, false);
            return;
        }
    }
    if (ui->lineEditNewPassword->text().isEmpty())
    {
        setValid(ui->lineEditNewPassword, false);
        return;
    }
    if (ui->lineEditNewPasswordRepeat->text().isEmpty())
    {
        setValid(ui->lineEditNewPasswordRepeat, false);
        return;
    }
    if (ui->lineEditNewPassword->text().compare(ui->lineEditNewPasswordRepeat->text()) != 0)
    {
        setValid(ui->lineEditNewPassword, false);
        setValid(ui->lineEditNewPasswordRepeat, false);
        return;
    }

    if (pactiveWallet->IsCrypted())
    {
        if(!pactiveWallet->ChangeWalletPassphrase(ui->lineEditOldPassword->text().toStdString().c_str(), ui->lineEditNewPassword->text().toStdString().c_str()))
        {
            setValid(ui->lineEditOldPassword, false);
            return;
        }
        burnLineEditMemory(ui->lineEditOldPassword);
        burnLineEditMemory(ui->lineEditNewPassword);
        burnLineEditMemory(ui->lineEditNewPasswordRepeat);
        Q_EMIT dismiss();
    }
    else
    {
        if(!pactiveWallet->EncryptWallet(ui->lineEditNewPassword->text().toStdString().c_str()))
        {
            setValid(ui->lineEditNewPassword, false);
            return;
        }
        pactiveWallet->Lock();
        burnLineEditMemory(ui->lineEditOldPassword);
        burnLineEditMemory(ui->lineEditNewPassword);
        burnLineEditMemory(ui->lineEditNewPasswordRepeat);
        Q_EMIT dismiss();
    }
}
