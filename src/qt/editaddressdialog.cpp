// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2020 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "editaddressdialog.h"
#include "ui_editaddressdialog.h"

#include "addresstablemodel.h"
#include "guiutil.h"

#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QPushButton>
#include "GuldenGUI.h"

EditAddressDialog::EditAddressDialog(Mode _mode, QWidget *parent, QString labelHint):
    QDialog(parent),
    ui(new Ui::EditAddressDialog),
    mapper(0),
    mode(_mode),
    model(0)
{
    ui->setupUi(this);

    GUIUtil::setupAddressWidget(ui->addressEdit, this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(GULDEN_DIALOG_CONFIRM_BUTTON_STYLE_NOMARGIN);
    ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Cancel"));
    ui->buttonBox->button(QDialogButtonBox::Reset)->setCursor(Qt::PointingHandCursor);
    ui->buttonBox->button(QDialogButtonBox::Reset)->setStyleSheet(GULDEN_DIALOG_CANCEL_BUTTON_STYLE_NOMARGIN);
    QObject::connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(reject()));

    QFrame* horizontalLine = new QFrame(this);
    horizontalLine->setFrameStyle(QFrame::HLine);
    horizontalLine->setFixedHeight(1);
    horizontalLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    horizontalLine->setStyleSheet(GULDEN_DIALOG_HLINE_STYLE_NOMARGIN);
    ui->verticalLayout->insertWidget(1, horizontalLine);
    
    if (labelHint.size() > 0)
    {
        ui->labelEdit->setText(labelHint);
        ui->addressEdit->setFocus();
    }

    //Need a minimum height otherwise our horizontal line gets hidden.
    setMinimumSize(400,200);

    switch(mode)
    {
        case NewReceivingAddress:
            setWindowTitle(tr("New receiving address"));
            ui->addressEdit->setEnabled(false);
            break;
        case NewSendingAddress:
            setWindowTitle(tr("New sending address"));
            break;
        case NewMiningAddress:
            setWindowTitle(tr("New mining address"));
            break;
        case EditReceivingAddress:
            setWindowTitle(tr("Edit receiving address"));
            ui->addressEdit->setEnabled(false);
            break;
        case EditSendingAddress:
            setWindowTitle(tr("Edit sending address"));
            break;
    }

    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
}

EditAddressDialog::~EditAddressDialog()
{
    delete ui;
}

void EditAddressDialog::setModel(AddressTableModel *_model)
{
    this->model = _model;
    if(!_model)
        return;

    mapper->setModel(_model);
    mapper->addMapping(ui->labelEdit, AddressTableModel::Label);
    mapper->addMapping(ui->addressEdit, AddressTableModel::Address);
}

void EditAddressDialog::loadRow(int row)
{
    mapper->setCurrentIndex(row);
}

bool EditAddressDialog::saveCurrentRow()
{
    if(!model)
        return false;

    switch(mode)
    {
        case NewReceivingAddress:
        case NewSendingAddress:
        {
            const auto type = (mode == NewSendingAddress) ? AddressTableModel::Send : AddressTableModel::Receive;
            address = model->addRow(ui->addressEdit->text(), ui->labelEdit->text(), ui->descriptionEdit->text(), type);
            break;
        }
        case NewMiningAddress:
        {
            address = model->addRow(ui->addressEdit->text(), ui->labelEdit->text(), ui->descriptionEdit->text(), AddressTableModel::Send);
            break;
        }
        case EditReceivingAddress:
        case EditSendingAddress:
        {
            if(mapper->submit())
            {
                address = ui->addressEdit->text();
            }
            break;
        }
    }
    return !address.isEmpty();
}

void EditAddressDialog::accept()
{
    if(!model)
        return;

    if(!saveCurrentRow())
    {
        switch(model->getEditStatus())
        {
            case AddressTableModel::OK:
                // Failed with unknown reason. Just reject.
                break;
            case AddressTableModel::NO_CHANGES:
                // No changes were made during edit operation. Just reject.
                break;
            case AddressTableModel::INVALID_ADDRESS:
                QMessageBox::warning(this, windowTitle(),
                    tr("The entered address \"%1\" is not a valid Gulden address.").arg(ui->addressEdit->text()),
                    QMessageBox::Ok, QMessageBox::Ok);
                break;
            case AddressTableModel::DUPLICATE_ADDRESS:
                // Special case, when called from the mining dialog we want to accept the address, even if its already in the address book with a different name.
                if (mode == NewMiningAddress)
                {
                    address = ui->addressEdit->text();
                    QDialog::accept();
                }
                else
                {
                    QMessageBox::warning(this, windowTitle(),
                        tr("The entered address \"%1\" is already in the address book.").arg(ui->addressEdit->text()),
                        QMessageBox::Ok, QMessageBox::Ok);
                }
                break;
            case AddressTableModel::WALLET_UNLOCK_FAILURE:
                QMessageBox::critical(this, windowTitle(),
                    tr("Could not unlock wallet."),
                    QMessageBox::Ok, QMessageBox::Ok);
                break;
            case AddressTableModel::KEY_GENERATION_FAILURE:
                QMessageBox::critical(this, windowTitle(),
                    tr("New key generation failed."),
                    QMessageBox::Ok, QMessageBox::Ok);
                break;

            }
        return;
    }
    QDialog::accept();
}

QString EditAddressDialog::getAddress() const
{
    return address;
}

void EditAddressDialog::setAddress(const QString &_address)
{
    this->address = _address;
    ui->addressEdit->setText(_address);
}
