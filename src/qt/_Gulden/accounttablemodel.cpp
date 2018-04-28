// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "accounttablemodel.h"
#include "walletmodel.h"
#include "bitcoinunits.h"

#include <wallet/wallet.h>
#include "validation.h" // For cs_main

const QString AccountTableModel::Active = "A";
const QString AccountTableModel::Inactive = "I";

AccountTableModel::AccountTableModel(CWallet *wallet, WalletModel *parent)
: QAbstractTableModel(parent)
, m_wallet(wallet)
{
    activeAccount = parent->getActiveAccount();
    connect(parent, SIGNAL(activeAccountChanged(CAccount*)), this, SLOT(activeAccountChanged(CAccount*)));
    connect(parent, SIGNAL(accountAdded(CAccount*)), this, SLOT(accountAdded(CAccount*)));
}

int AccountTableModel::rowCount(const QModelIndex& parent) const
{
    LOCK(m_wallet->cs_wallet);

    if (!parent.isValid())
    {
        if (m_wallet)
            return m_wallet->mapAccountLabels.size();
    }
    return 0;
}

int AccountTableModel::columnCount(const QModelIndex & parent) const
{
    return 2;
}

QVariant AccountTableModel::data(const QModelIndex& index, int role) const
{
    if (!m_wallet)
        return QVariant();

    {
        LOCK(m_wallet->cs_wallet);

        if (index.row() < 0 || (unsigned int)index.row() >= m_wallet->mapAccountLabels.size())
        {
            return QVariant();
        }
    }

    CAccount* account = NULL;
    std::string accountLabel;
    boost::uuids::uuid accountUUID;
    {
        LOCK(m_wallet->cs_wallet);
        auto iter = m_wallet->mapAccountLabels.begin();
        std::advance(iter, index.row());
        if (m_wallet->mapAccounts.count(iter->first) == 0)
            return QVariant();

        account = m_wallet->mapAccounts[iter->first];
        accountLabel = iter->second;
        accountUUID = iter->first;
    }

    if  (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
        {
            return QString::fromStdString(accountLabel.c_str());
        }
        if (index.column() == 1)
        {
            CAmount balance = pactiveWallet->GetLegacyBalance(ISMINE_SPENDABLE, 0, &accountUUID);
            return BitcoinUnits::format(BitcoinUnits::Unit::BTC, balance, false, BitcoinUnits::separatorAlways, 2);
        }
    }
    else if (role == TypeRole)
    {
        return GetAccountTypeString(account->m_Type).c_str();
    }
    else if (role == SubTypeRole)
    {
        return GetAccountSubTypeString(account->m_SubType).c_str();
    }
    else if (role == AvailableBalanceRole)
    {
        return (qlonglong)m_wallet->GetLegacyBalance(ISMINE_SPENDABLE, 0, &accountUUID );
    }
    else if (role == ActiveAccountRole)
    {
        if (account == activeAccount)
        {
            return Active;
        }
        else
        {
            return Inactive;
        }
    }
    else if(role == SelectedAccountRole)
    {
        return QString::fromStdString(getUUIDAsString(accountUUID));
    }
    else if(role == Qt::TextAlignmentRole)
    {
        switch(index.column())
        {
            case 0:
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            case 1:
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QVariant();
}

void AccountTableModel::activeAccountChanged(CAccount* account)
{
    LogPrintf("AccountTableModel::activeAccountChanged\n");

    // Unfortunately because of "GetLegacyBalance" some of our child models might cause cs_main to lock
    // Because of lock order always being cs_main before cs_wallet (to prevent deadlock) we are forced to lock cs_main here.
    LOCK2(cs_main, m_wallet->cs_wallet);

    activeAccount = account;
    //fixme: Technically we can emit for just the two rows here.
    beginResetModel();
    endResetModel();
}

void AccountTableModel::accountAdded(CAccount* account)
{
    // Unfortunately because of "GetLegacyBalance" some of our child models might cause cs_main to lock
    // Because of lock order always being cs_main before cs_wallet (to prevent deadlock) we are forced to lock cs_main here.
    LOCK2(cs_main, m_wallet->cs_wallet);

    beginResetModel();
    endResetModel();
    //fixme: We should instead use something like the below...
    //int pos =  std::distance(m_wallet->mapAccountLabels.begin(), m_wallet->mapAccountLabels.find(account->getUUID()));
    //beginInsertRows(index(0, 0, QModelIndex()), pos, pos);
    //endInsertRows();
}
