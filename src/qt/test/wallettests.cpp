#include "wallettests.h"

#include "qt/amountfield.h"
#include "qt/callback.h"
#include "qt/optionsmodel.h"
#include "qt/qvalidatedlineedit.h"
#include "qt/sendcoinsdialog.h"
#include "qt/guldensendcoinsentry.h"
#include "qt/transactiontablemodel.h"
#include "qt/transactionview.h"
#include "qt/walletmodel.h"
#include "test/test.h"
#include "validation/validation.h"
#include "wallet/wallet.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QProxyStyle>

namespace
{
//fixme: (FUT) - We don't currently have fee bumping enabled in the UI
#if 0

//! Press "Ok" button in message box dialog.
void ConfirmMessage(QString* text = nullptr)
{
    QTimer::singleShot(0, makeCallback([text](Callback* callback) {
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (widget->inherits("QMessageBox")) {
                QMessageBox* messageBox = qobject_cast<QMessageBox*>(widget);
                if (text) *text = messageBox->text();
                messageBox->defaultButton()->click();
            }
        }
        delete callback;
    }), SLOT(call()));
}
#endif

//! Press "Yes" or "Cancel" buttons in modal send confirmation dialog.
void ConfirmSend(QString* text = nullptr, bool cancel = false)
{
    QTimer::singleShot(0, makeCallback([text, cancel](Callback* callback)
    {
        for (QWidget* widget : QApplication::topLevelWidgets())
        {
            if (widget->objectName() == ("SendConfirmationDialog"))
            {
                QDialog* dialog = qobject_cast<QDialog*>(widget);
                if (text)
                    *text = dialog->findChild<QLabel*>("labelDialogMessage")->text();

                QAbstractButton* button = dialog->findChild<QAbstractButton*>(cancel ? "dialogCancelButton" : "dialogConfirmButton");
                button->setEnabled(true);
                button->click();
            }
        }
        delete callback;
    }), SLOT(call()));
}

//! Send coins to address and return txid.
uint256 SendCoins(CWallet& wallet, SendCoinsDialog& sendCoinsDialog, const CNativeAddress& address, CAmount amount, bool rbf)
{
    QVBoxLayout* entries = sendCoinsDialog.findChild<QVBoxLayout*>("entries");
    GuldenSendCoinsEntry* entry = qobject_cast<GuldenSendCoinsEntry*>(entries->itemAt(0)->widget());
    entry->findChild<QLineEdit*>("receivingAddress")->setText(QString::fromStdString(address.ToString()));
    entry->findChild<GuldenAmountField*>("payAmount")->setAmount(amount);
    /*sendCoinsDialog.findChild<QFrame*>("frameFee")
        ->findChild<QFrame*>("frameFeeSelection")
        ->findChild<QCheckBox*>("optInRBF")
        ->setCheckState(rbf ? Qt::Checked : Qt::Unchecked);*/
    uint256 txid;
    boost::signals2::scoped_connection c(wallet.NotifyTransactionChanged.connect([&txid](CWallet*, const uint256& hash, ChangeType status, bool) {
        if (status == CT_NEW) txid = hash;
    }));
    ConfirmSend();
    QMetaObject::invokeMethod(&sendCoinsDialog, "on_sendButton_clicked");
    return txid;
}

//! Find index of txid in transaction list.
QModelIndex FindTx(const QAbstractItemModel& model, const uint256& txid)
{
    QString hash = QString::fromStdString(txid.ToString());
    int rows = model.rowCount({});
    for (int row = 0; row < rows; ++row) {
        QModelIndex index = model.index(row, 0, {});
        if (model.data(index, TransactionTableModel::TxHashRole) == hash) {
            return index;
        }
    }
    return {};
}

//fixme: (FUT) - We don't currently have fee bumping enabled in the UI
#if 0
//! Invoke bumpfee on txid and check results.
void BumpFee(TransactionView& view, const uint256& txid, bool expectDisabled, std::string expectError, bool cancel)
{
    QTableView* table = view.findChild<QTableView*>("transactionView");
    QModelIndex index = FindTx(*table->selectionModel()->model(), txid);
    QVERIFY2(index.isValid(), "Could not find BumpFee txid");

    // Select row in table, invoke context menu, and make sure bumpfee action is
    // enabled or disabled as expected.
    QAction* action = view.findChild<QAction*>("bumpFeeAction");
    table->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    action->setEnabled(expectDisabled);
    RequestContextMenu(table);
    QCOMPARE(action->isEnabled(), !expectDisabled);

    action->setEnabled(true);
    QString text;
    if (expectError.empty()) {
        ConfirmSend(&text, cancel);
    } else {
        ConfirmMessage(&text);
    }
    action->trigger();
    QVERIFY(text.indexOf(QString::fromStdString(expectError)) != -1);
}
#endif

//! Simple qt wallet tests.
//
// Test widgets can be debugged interactively calling show() on them and
// manually running the event loop, e.g.:
//
//     sendCoinsDialog.show();
//     QEventLoop().exec();
//
// This also requires overriding the default minimal Qt platform:
//
//     src/qt/test/test_gulden-qt -platform xcb      # Linux
//     src/qt/test/test_gulden-qt -platform windows  # Windows
//     src/qt/test/test_gulden-qt -platform cocoa    # macOS
void TestSendCoins()
{
    // Set up wallet and chain with 105 blocks (5 mature blocks for spending).
    TestChain100Setup test;
    CScript scriptPubKey = GetScriptForRawPubKey(test.coinbaseKey.GetPubKey());
    std::shared_ptr<CReserveKeyOrScript> reservedScript = std::make_shared<CReserveKeyOrScript>(scriptPubKey);
    for (int i = 0; i < 5; ++i) {
        test.CreateAndProcessBlock({}, reservedScript);
    }
    bitdb.MakeMock();
    fNoUI = true; // triggers recovery phrase generation
    CWallet* wallet = CWallet::CreateWalletFromFile("wallet_test.dat");
    WalletLoadState loadState;
    wallet->LoadWallet(loadState);
    {
        LOCK(wallet->cs_wallet);

        wallet->importPrivKey(test.coinbaseKey, "Imported key");
    }
    wallet->ScanForWalletTransactions(chainActive.Genesis(), true);
    wallet->SetBroadcastTransactions(true);

    // Create widgets for sending coins and listing transactions.
    std::unique_ptr<const QStyle> platformStyle(new QProxyStyle("windows"));
    SendCoinsDialog sendCoinsDialog(platformStyle.get());
    TransactionView transactionView;
    OptionsModel optionsModel;
    WalletModel walletModel(platformStyle.get(), wallet, &optionsModel);
    sendCoinsDialog.setModel(&walletModel);
    transactionView.setModel(&walletModel);

    // Send two transactions, and verify they are added to transaction list.
    TransactionTableModel* transactionTableModel = walletModel.getTransactionTableModel();
    QCOMPARE(transactionTableModel->rowCount({}), 105);
    uint256 txid1 = SendCoins(*wallet, sendCoinsDialog, CNativeAddress(CKeyID()), 5 * COIN, false /* rbf */);
    uint256 txid2 = SendCoins(*wallet, sendCoinsDialog, CNativeAddress(CKeyID()), 10 * COIN, true /* rbf */);
    QCOMPARE(transactionTableModel->rowCount({}), 107);
    QVERIFY(FindTx(*transactionTableModel, txid1).isValid());
    QVERIFY(FindTx(*transactionTableModel, txid2).isValid());

    //fixme: (FUT) - We don't currently have fee bumping enabled in the UI
    #if 0
    // Call bumpfee. Test disabled, canceled, enabled, then failing cases.
    BumpFee(transactionView, txid1, true /* expect disabled */, "not BIP 125 replaceable" /* expected error */, false /* cancel */);
    BumpFee(transactionView, txid2, false /* expect disabled */, {} /* expected error */, true /* cancel */);
    BumpFee(transactionView, txid2, false /* expect disabled */, {} /* expected error */, false /* cancel */);
    BumpFee(transactionView, txid2, true /* expect disabled */, "already bumped" /* expected error */, false /* cancel */);
    #endif

    wallet->Flush(true);
    delete wallet;

    bitdb.Flush(true);
    bitdb.Reset();
}

}

void WalletTests::walletTests()
{
    TestSendCoins();
}
