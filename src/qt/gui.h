// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_QT_GUI_H
#define GULDEN_QT_GUI_H

#if defined(HAVE_CONFIG_H)
#include "config/gulden-config.h"
#endif

#include "amount.h"

#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QSystemTrayIcon>

#include "_Gulden/GuldenGUI.h"
#include "validation.h"

#include <boost/uuid/uuid.hpp>

class ClientModel;
class NetworkStyle;
class Notificator;
class OptionsModel;
class PlatformStyle;
class RPCConsole;
class SendCoinsRecipient;
class UnitDisplayStatusBarControl;
class WalletFrame;
class WalletModel;
class HelpMessageDialog;
class ModalOverlay;
class AccountSummaryWidget;

class CWallet;

QT_BEGIN_NAMESPACE
class QAction;
class QProgressBar;
class QProgressDialog;
QT_END_NAMESPACE

//Needed for QVariant.
Q_DECLARE_METATYPE(boost::uuids::uuid);

/**
  Gulden GUI main class. This class represents the main window of the Gulden UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
class GUI : public QMainWindow
{
    Q_OBJECT

public:
    static const QString DEFAULT_WALLET;
    static const std::string DEFAULT_UIPLATFORM;

    explicit GUI(const PlatformStyle *platformStyle, const NetworkStyle *networkStyle, QWidget *parent = 0);
    ~GUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel *clientModel);
#ifdef ENABLE_WALLET
    /** Set the wallet model.
        The wallet model represents a Gulden wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    bool addWallet(const QString& name, WalletModel *walletModel);
    bool setCurrentWallet(const QString& name);
    void removeAllWallets();
    void updateUIForBlockTipChange();
#endif // ENABLE_WALLET
    bool enableWallet;

protected:
    void resizeEvent(QResizeEvent* event);
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

public:
    WalletFrame *walletFrame;
private:
    ClientModel *clientModel;

    UnitDisplayStatusBarControl *unitDisplayControl;
    QLabel *labelWalletEncryptionIcon;
    QLabel *labelWalletHDStatusIcon;
    QLabel *connectionsControl;
    QLabel *labelBlocksIcon;
    QLabel *progressBarLabel;
    QProgressBar *progressBar;
    QProgressDialog *progressDialog;

    QMenuBar *appMenuBar;
    QAction* witnessDialogAction;
    QAction *overviewAction;
    QAction *historyAction;
    QAction *quitAction;
    QAction *sendCoinsAction;
    QAction *sendCoinsMenuAction;
    QAction *usedSendingAddressesAction;
    QAction *usedReceivingAddressesAction;
    QAction *aboutAction;
    QAction *receiveCoinsAction;
    QAction *receiveCoinsMenuAction;
    QAction *optionsAction;
    QAction *toggleHideAction;
    QAction *encryptWalletAction;
    QAction *backupWalletAction;
    QAction *changePassphraseAction;
    QAction *aboutQtAction;
    QAction *openRPCConsoleAction;
    QAction *openAction;
    QAction *showHelpMessageAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QMenu *settingsMenu;
    Notificator *notificator;
    RPCConsole *rpcConsole;
    HelpMessageDialog *helpMessageDialog;
    ModalOverlay *modalOverlay;
    AccountSummaryWidget* accountSummaryWidget;

    /** Keep track of previous number of blocks, to detect progress */
    int prevBlocks;
    int spinnerFrame;

    const PlatformStyle *platformStyle;

    /** Create the main UI actions. */
    void createActions();
    /** Create the menu bar and sub-menus. */
    void createMenuBar();
    /** Create the toolbars */
    void createToolBars();
    /** Create system tray icon and notification */
    void createTrayIcon(const NetworkStyle *networkStyle);
    /** Create system tray menu (or setup the dock menu) */
    void createTrayIconMenu();

    /** Enable or disable all wallet-related actions */
    void setWalletActionsEnabled(bool enabled);

    /** Connect core signals to GUI client */
    void subscribeToCoreSignals();
    /** Disconnect core signals from GUI client */
    void unsubscribeFromCoreSignals();

    QFrame *frameBlocks;
    GuldenGUI* m_pGuldenImpl;
    friend class GuldenGUI;

    /** Update UI with latest network info from model. */
    void updateNetworkState();

    void updateHeadersSyncProgressLabel(int current, int total);

Q_SIGNALS:
    /** Signal raised when a URI was entered or dragged to the GUI */
    void receivedURI(const QString &uri);

public Q_SLOTS:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set network state shown in the UI */
    void setNetworkActive(bool networkActive);
    /** Set number of blocks and last block date shown in the UI */
    void setNumBlocks(int count, const QDateTime& blockDate, double nVerificationProgress, bool headers);
    /** Set number of headers and headers sync progress in the UI */
    void setNumHeaders(int current, int total);

    /** Notify the user of an event from the core network or transaction handling code.
       @param[in] title     the message box / notification title
       @param[in] message   the displayed text
       @param[in] style     modality and style definitions (icon and used buttons - buttons only for message boxes)
                            @see CClientUIInterface::MessageBoxFlags
       @param[in] ret       pointer to a bool that will be modified to whether Ok was clicked (modal only)
    */
    void message(const QString &title, const QString &message, unsigned int style, bool *ret = NULL);

    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

#ifdef ENABLE_WALLET
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    void setEncryptionStatus(int status);

    bool handlePaymentRequest(const SendCoinsRecipient& recipient);

    /** Show incoming transaction notification for new transactions. */
    void incomingTransaction(const QString& date, int unit, const CAmount& amountReceived, const CAmount& amountSent, const QString& type, const QString& address, const QString& account, const QString& label);
#endif // ENABLE_WALLET

private Q_SLOTS:
#ifdef ENABLE_WALLET
    /** Switch to witness overview page */
    void showWitnessDialog();
    /** Switch to overview (home) page */
    void gotoOverviewPage();
    /** Switch to history (transactions) page */
    void gotoHistoryPage();
    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();
    /** Switch to send coins page */
    void gotoSendCoinsPage(QString addr = "");

    /** Show open dialog */
    void openClicked();
#endif // ENABLE_WALLET
    /** Show configuration dialog */
    void optionsClicked();
    /** Show about dialog */
    void aboutClicked();
    /** Show debug window */
    void showDebugWindow();
    /** Show debug window and set focus to the console */
    void showDebugWindowActivateConsole();
    /** Show help message dialog */
    void showHelpMessageClicked();
#ifndef Q_OS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif

    /** Show window if hidden, unminimize when minimized, rise when obscured or show if hidden and fToggleHidden is true */
    void showNormalIfMinimized(bool fToggleHidden = false);
    /** Simply calls showNormalIfMinimized(true) for use in SLOT() macro */
    void toggleHidden();

    /** called by a timer to check if fRequestShutdown has been set **/
    void detectShutdown();

    /** Show progress dialog e.g. for verifychain */
    void showProgress(const QString &title, int nProgress);

    /** When hideTrayIcon setting is changed in OptionsModel hide or show the icon accordingly. */
    void setTrayIconVisible(bool);

    /** Toggle networking */
    void toggleNetworkActive();

    void showModalOverlay();
};

class UnitDisplayStatusBarControl : public QLabel
{
    Q_OBJECT

public:
    explicit UnitDisplayStatusBarControl(const PlatformStyle *platformStyle);
    /** Lets the control know about the Options Model (and its signals) */
    void setOptionsModel(OptionsModel *optionsModel);

protected:
    /** So that it responds to left-button clicks */
    void mousePressEvent(QMouseEvent *event);

private:
    OptionsModel *optionsModel;
    QMenu* menu;

    /** Shows context menu with Display Unit options by the mouse coordinates */
    void onDisplayUnitsClicked(const QPoint& point);
    /** Creates context menu, its actions, and wires up all the relevant signals for mouse events. */
    void createContextMenu();

private Q_SLOTS:
    /** When Display Units are changed on OptionsModel it will refresh the display text of the control on the status bar */
    void updateDisplayUnit(int newUnits);
    /** Tells underlying optionsModel to update its current display unit. */
    void onMenuSelection(QAction* action);
};

#endif // GULDEN_QT_GUI_H
