// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_ACCOUNTTABLEMODEL_H
#define GULDEN_ACCOUNTTABLEMODEL_H

#include <QAbstractTableModel>
#include <map>

class CWallet;
class CAccount;
class WalletModel;

class AccountTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum AccountTableRoles { StateRole = Qt::UserRole + 1, TypeRole, ActiveAccountRole, SelectedAccountRole, AvailableBalanceRole };
    enum ColumnIndex { Label, Balance };

    static const QString Normal;
    static const QString Active;
    static const QString Inactive;

    explicit AccountTableModel(CWallet *wallet, WalletModel *parent = 0);
    virtual ~AccountTableModel();
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

private:
    CWallet* m_wallet;
    CAccount* activeAccount;

public Q_SLOTS:
    void activeAccountChanged(CAccount*);
    void accountAdded(CAccount*);
};

#endif // GULDEN_ACCOUNTTABLEMODEL_H
