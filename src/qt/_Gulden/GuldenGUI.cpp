// Copyright (c) 2015-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#if defined(HAVE_CONFIG_H)
#include "config/gulden-config.h"
#endif

#include "GuldenGUI.h"
#include "gui.h"
#include "platformstyle.h"
#include "units.h"
#include "clickablelabel.h"
#include "receivecoinsdialog.h"
#include "validation.h"
#include "guiutil.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSettings>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QProxyStyle>
#include <QLineEdit>
#include <QTextEdit>
#include <QCollator>

#include <_Gulden/accountsummarywidget.h>
#include <_Gulden/newaccountdialog.h>
#include <_Gulden/importprivkeydialog.h>
#include <_Gulden/exchangeratedialog.h>
#include <_Gulden/accountsettingsdialog.h>
#include <_Gulden/witnessdialog.h>
#include <Gulden/util.h>
#include <consensus/consensus.h>
#include "sendcoinsdialog.h"
#include "wallet/wallet.h"
#include "walletframe.h"
#include "walletview.h"
#include "utilmoneystr.h"
#include "passwordmodifydialog.h"
#include "backupdialog.h"
#include "welcomedialog.h"
#include "ticker.h"
#include "nockssettings.h"
#include "units.h"
#include "optionsmodel.h"
#include "askpassphrasedialog.h"


//Font sizes - NB! We specifically use 'px' and not 'pt' for all font sizes, as qt scales 'pt' dynamically in a way that makes our fonts unacceptably small on OSX etc. it doesn't do this with px so we use px instead.
//QString CURRENCY_DECIMAL_FONT_SIZE = "11px"; // For .00 in currency and PND text.
//QString BODY_FONT_SIZE = "12px"; // Standard body font size used in 'most places'.
//QString CURRENCY_FONT_SIZE = "13px"; // For currency
//QString TOTAL_FONT_SIZE = "15px"; // For totals and account names
//QString HEADER_FONT_SIZE = "16px"; // For large headings

const char* LOGO_FONT_SIZE = "28px";
const char* TOOLBAR_FONT_SIZE = "15px";
const char* BUTTON_FONT_SIZE = "15px";
const char* SMALL_BUTTON_FONT_SIZE = "14px";
const char* MINOR_LABEL_FONT_SIZE = "12px";

//Colors
const char* ACCENT_COLOR_1 = "#007aff";
const char* ACCENT_COLOR_2 = "#0067d9";
const char* TEXT_COLOR_1 = "#999";
const char* COLOR_VALIDATION_FAILED = "#FF8080";


//Toolbar constants
unsigned int sideBarWidthNormal = 300;
unsigned int sideBarWidth = sideBarWidthNormal;
const unsigned int horizontalBarHeight = 60;
const char* SIDE_BAR_WIDTH = "300px";
//Extended width for large balances.
unsigned int sideBarWidthExtended = 340;
const char* SIDE_BAR_WIDTH_EXTENDED = "340px";

GuldenProxyStyle::GuldenProxyStyle()
: QProxyStyle("windows") //Use the same style on all platforms to simplify skinning
{
    altDown = false;
}

void GuldenProxyStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    // Only draw underline hints on buttons etc. if the alt key is pressed.
    if (altDown)
    {
        alignment |= Qt::TextShowMnemonic;
        alignment &= ~(Qt::TextHideMnemonic);
    }
    else
    {
        alignment |= Qt::TextHideMnemonic;
        alignment &= ~(Qt::TextShowMnemonic);
    }

    QProxyStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}


bool GuldenEventFilter::eventFilter(QObject *obj, QEvent *evt)
{
    if (evt->type() == QEvent::KeyPress || evt->type() == QEvent::KeyRelease)
    {
        QKeyEvent *KeyEvent = (QKeyEvent*)evt;
        if (KeyEvent->key() == Qt::Key_Alt)
        {
            guldenProxyStyle->altDown = (evt->type() == QEvent::KeyPress);
            parentObject->repaint();
        }
    }
    else if(evt->type() == QEvent::ApplicationDeactivate || evt->type() == QEvent::ApplicationStateChange || evt->type() == QEvent::Leave)
    {
        if(guldenProxyStyle->altDown)
        {
            guldenProxyStyle->altDown = false;
            parentObject->repaint();
        }
    }
    return QObject::eventFilter(obj, evt);
}


void setValid(QWidget* control, bool validity)
{
    control->setProperty("valid", validity);
    control->style()->unpolish(control);
    control->style()->polish(control);
}

void burnLineEditMemory(QLineEdit* edit)
{
    // Attempt to overwrite text so that they do not linger around in memory
    edit->setText(QString(" ").repeated(edit->text().size()));
    edit->clear();
}

void burnTextEditMemory(QTextEdit* edit)
{
    // Attempt to overwrite text so that they do not linger around in memory
    edit->setText(QString(" ").repeated(edit->toPlainText().length()));
    edit->clear();
}

static void NotifyRequestUnlockS(GuldenGUI* parent, CWallet* wallet, std::string reason)
{
    QMetaObject::invokeMethod(parent, "NotifyRequestUnlock", Qt::QueuedConnection, Q_ARG(void*, wallet), Q_ARG(QString, QString::fromStdString(reason)));
}

static void NotifyRequestUnlockWithCallbackS(GuldenGUI* parent, CWallet* wallet, std::string reason, std::function<void (void)> callback)
{
    QMetaObject::invokeMethod(parent, "NotifyRequestUnlockWithCallback", Qt::QueuedConnection, Q_ARG(void*, wallet), Q_ARG(QString, QString::fromStdString(reason)), Q_ARG(std::function<void (void)>, callback));
}

GuldenGUI::GuldenGUI( GUI* pImpl )
: QObject()
, m_pImpl( pImpl )
, accountBar( NULL )
, guldenBar( NULL )
, spacerBarL( NULL )
, spacerBarR( NULL )
, tabsBar( NULL )
, accountInfoBar( NULL )
, statusBar( NULL )
, menuBarSpaceFiller( NULL )
, balanceContainer( NULL )
, welcomeScreen( NULL )
, accountScrollArea( NULL )
, toolsMenu( NULL )
, importPrivateKeyAction( NULL )
, rescanAction( NULL )
, currencyAction ( NULL )
, dialogNewAccount( NULL )
, dialogAccountSettings( NULL )
, dialogBackup( NULL )
, dialogPasswordModify( NULL )
, dialogExchangeRate ( NULL )
, cacheCurrentWidget( NULL )
, ticker( NULL )
, nocksSettings( NULL )
, labelBalance ( NULL )
, labelBalanceForex ( NULL )
, passwordAction( NULL )
, backupAction( NULL )
, optionsModel( NULL )
, receiveAddress( NULL )
, guldenStyle (NULL)
, guldenEventFilter (NULL)
{
    ticker = new CurrencyTicker( this );
    nocksSettings = new NocksSettings( this );

    //Start the ticker polling off - after the initial call the ticker will schedule the subsequent ones internally.
    ticker->pollTicker();
    nocksSettings->pollSettings();

    connect( ticker, SIGNAL( exchangeRatesUpdated() ), this, SLOT( updateExchangeRates() ) );

    uiInterface.RequestUnlock.connect(boost::bind(NotifyRequestUnlockS, this, _1, _2));
    uiInterface.RequestUnlockWithCallback.connect(boost::bind(NotifyRequestUnlockWithCallbackS, this, _1, _2, _3));
}


bool requestUnlockDialogAlreadyShowing=false;
void GuldenGUI::NotifyRequestUnlock(void* wallet, QString reason)
{
    if (!requestUnlockDialogAlreadyShowing)
    {
        requestUnlockDialogAlreadyShowing = true;
        LogPrintf("NotifyRequestUnlock\n");
        AskPassphraseDialog dlg(AskPassphraseDialog::Unlock, m_pImpl, reason);
        dlg.setModel(new WalletModel(NULL, (CWallet*)wallet, NULL, NULL));
        dlg.exec();
        requestUnlockDialogAlreadyShowing = false;
    }
}

void GuldenGUI::NotifyRequestUnlockWithCallback(void* wallet, QString reason, std::function<void (void)> successCallback)
{
    if (!requestUnlockDialogAlreadyShowing)
    {
        requestUnlockDialogAlreadyShowing = true;
        LogPrintf("NotifyRequestUnlockWithCallback\n");
        AskPassphraseDialog dlg(AskPassphraseDialog::Unlock, m_pImpl, reason);
        dlg.setModel(new WalletModel(NULL, (CWallet*)wallet, NULL, NULL));
        int result = dlg.exec();
        if(result == QDialog::Accepted)
            successCallback();
        requestUnlockDialogAlreadyShowing = false;
    }
}

void GuldenGUI::handlePaymentAccepted()
{
    refreshTabVisibilities();
}

GuldenGUI::~GuldenGUI()
{
    if (guldenEventFilter)
    {
        m_pImpl->removeEventFilter(guldenEventFilter);
        delete guldenEventFilter;
    }
}

void GuldenGUI::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    balanceCached = balance;
    unconfirmedBalanceCached = unconfirmedBalance;
    immatureBalanceCached = immatureBalance;
    watchOnlyBalanceCached = watchOnlyBalance;
    watchUnconfBalanceCached = watchUnconfBalance;
    watchImmatureBalanceCached = watchImmatureBalance;

    if (!labelBalance || !labelBalanceForex)
        return;

    CAmount displayBalance = balance + unconfirmedBalance + immatureBalance;
    labelBalance->setText(GuldenUnits::format(GuldenUnits::NLG, displayBalance, false, GuldenUnits::separatorStandard, 2));
    if (displayBalance > 0 && optionsModel)
    {
        labelBalanceForex->setText(QString("(") + QString::fromStdString(CurrencySymbolForCurrencyCode(optionsModel->guldenSettings->getLocalCurrency().toStdString())) + QString("\u2009") + GuldenUnits::format(GuldenUnits::NLG, ticker->convertGuldenToForex(displayBalance, optionsModel->guldenSettings->getLocalCurrency().toStdString()), false, GuldenUnits::separatorAlways, 2) + QString(")"));
        if (labelBalance->isVisible())
            labelBalanceForex->setVisible(true);
    }
    else
    {
        labelBalanceForex->setVisible(false);
    }

    if (accountScrollArea && displayBalance > 999999 * COIN && sideBarWidth != sideBarWidthExtended)
    {
        sideBarWidth = sideBarWidthExtended;
        doApplyStyleSheet();
        resizeToolBarsGulden();
    }
    else if (accountScrollArea && displayBalance < 999999 * COIN && sideBarWidth == sideBarWidthExtended)
    {
        sideBarWidth = sideBarWidthNormal;
        doApplyStyleSheet();
        resizeToolBarsGulden();
    }

    labelBalance->setToolTip("");
    if (immatureBalance>0 || unconfirmedBalance>0)
    {
        QString toolTip;
        if (unconfirmedBalance > 0)
        {
            toolTip += tr("Pending confirmation: %1").arg(GuldenUnits::formatWithUnit(GuldenUnits::NLG, unconfirmedBalance, false, GuldenUnits::separatorStandard, 2));
        }
        if (immatureBalance > 0)
        {
            if (!toolTip.isEmpty())
                toolTip += "\n";
            toolTip += tr("Pending maturity: %1").arg(GuldenUnits::formatWithUnit(GuldenUnits::NLG, immatureBalance, false, GuldenUnits::separatorStandard, 2));
        }
        labelBalance->setToolTip(toolTip);
    }
}

void GuldenGUI::updateExchangeRates()
{
    setBalance(balanceCached, unconfirmedBalanceCached, immatureBalanceCached, watchOnlyBalanceCached, watchUnconfBalanceCached, watchImmatureBalanceCached);
}

void GuldenGUI::requestRenewWitness(CAccount* funderAccount)
{
    CAccount* targetWitnessAccount = pactiveWallet->getActiveAccount();

    std::string strError;
    CMutableTransaction tx(CURRENT_TX_VERSION_POW2);
    CReserveKey changeReserveKey(pactiveWallet, funderAccount, KEYCHAIN_EXTERNAL);
    CAmount txFee;
    if (!pactiveWallet->PrepareRenewWitnessAccountTransaction(funderAccount, targetWitnessAccount, changeReserveKey, tx, txFee, strError))
    {
        //fixme: (2.0) Improve error message
        QString message = QString::fromStdString(strError.c_str());
        QDialog* d = createDialog(m_pImpl, message, tr("Okay"), QString(""), 400, 180);
        d->exec();
    }

    QString questionString = tr("Renewing witness account will incur a transaction fee: ");
    questionString.append("<span style='color:#aa0000;'>");
    questionString.append(GuldenUnits::formatHtmlWithUnit(optionsModel->getDisplayUnit(), txFee));
    questionString.append("</span> ");
    QDialog* d = createDialog(m_pImpl, questionString, tr("Send"), tr("Cancel"), 600, 360);

    int result = d->exec();
    if(result != QDialog::Accepted)
    {
        return;
    }

    {
        LOCK2(cs_main, pactiveWallet->cs_wallet);
        if (!pactiveWallet->SignAndSubmitTransaction(changeReserveKey, tx, strError))
        {
            //fixme: (2.0) Improve error message
            QString message = QString::fromStdString(strError.c_str());
            QDialog* d = createDialog(m_pImpl, message, tr("Okay"), QString(""), 400, 180);
            d->exec();
        }
    }

    // Clear the failed flag in UI, and remove the 'renew' button for immediate user feedback.
    targetWitnessAccount->SetWarningState(AccountStatus::WitnessPending);
    static_cast<const CGuldenWallet*>(pactiveWallet)->NotifyAccountWarningChanged(pactiveWallet, targetWitnessAccount);
    m_pImpl->walletFrame->currentWalletView()->witnessDialogPage->update();
}

void GuldenGUI::requestFundWitness(CAccount* funderAccount)
{
    CAccount* targetWitnessAccount = pactiveWallet->getActiveAccount();
    pactiveWallet->setActiveAccount(funderAccount);
    refreshAccountControls();
    m_pImpl->gotoSendCoinsPage();
    m_pImpl->walletFrame->currentWalletView()->sendCoinsPage->gotoWitnessTab(targetWitnessAccount);
}

void GuldenGUI::requestEmptyWitness()
{
    CAccount* fromWitnessAccount = pactiveWallet->getActiveAccount();
    CAmount availableAmount = pactiveWallet->GetBalance(fromWitnessAccount, false, true);
    if (availableAmount > 0)
    {
        m_pImpl->walletFrame->gotoSendCoinsPage();
        m_pImpl->walletFrame->currentWalletView()->sendCoinsPage->setAmount(availableAmount);
    }
    else
    {
        QString message = tr("The funds in this account are currently locked for witnessing and cannot be transfered, please wait until lock expires or for earnings to accumulate before trying again.");
        QDialog* d = createDialog(m_pImpl, message, tr("Okay"), QString(""), 400, 180);
        d->exec();
    }
}

void GuldenGUI::setOptionsModel(OptionsModel* optionsModel_)
{
    optionsModel = optionsModel_;
    ticker->setOptionsModel(optionsModel);
    optionsModel->setTicker(ticker);
    optionsModel->setNocksSettings(nocksSettings);
    m_pImpl->accountSummaryWidget->setOptionsModel(optionsModel);
    connect( optionsModel->guldenSettings, SIGNAL(  localCurrencyChanged(QString) ), this, SLOT( updateExchangeRates() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    updateExchangeRates();
}

void GuldenGUI::createMenusGulden()
{
    toolsMenu = m_pImpl->appMenuBar->addMenu(tr("&Tools"));

    importPrivateKeyAction = new QAction(m_pImpl->platformStyle->TextColorIcon(":/Gulden/import"), tr("&Import key"), this);
    importPrivateKeyAction->setStatusTip(tr("Import a private key address"));
    importPrivateKeyAction->setCheckable(false);
    toolsMenu->addAction(importPrivateKeyAction);
    connect(importPrivateKeyAction, SIGNAL(triggered()), this, SLOT(promptImportPrivKey()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));

    rescanAction = new QAction(m_pImpl->platformStyle->TextColorIcon(":/Gulden/rescan"), tr("&Rescan transactions"), this);
    rescanAction->setStatusTip(tr("Rescan the blockchain looking for any missing transactions"));
    rescanAction->setCheckable(false);
    toolsMenu->addAction(rescanAction);
    connect(rescanAction, SIGNAL(triggered()), this, SLOT(promptRescan()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));

    currencyAction = new QAction(m_pImpl->platformStyle->TextColorIcon(":/icons/options"), tr("&Select currency"), this);
    currencyAction->setStatusTip(tr("Rescan the blockchain looking for any missing transactions"));
    currencyAction->setCheckable(false);
    m_pImpl->settingsMenu->addAction(currencyAction);
    connect(currencyAction, SIGNAL(triggered()), this, SLOT(showExchangeRateDialog()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));
}

void GuldenGUI::createToolBarsGulden()
{
    //Filler for right of menu bar.
    #ifndef MAC_OSX
    menuBarSpaceFiller = new QFrame( m_pImpl );
    menuBarSpaceFiller->setObjectName( "menuBarSpaceFiller" );
    menuBarSpaceFiller->move(sideBarWidth, 0);
    menuBarSpaceFiller->setFixedSize(20000, 21);
    #endif

    //Add the 'Account bar' - vertical bar on the left
    accountBar = new QToolBar( QCoreApplication::translate( "toolbar", "Account toolbar" ) );
    accountBar->setObjectName( "account_bar" );
    accountBar->setMovable( false );
    accountBar->setFixedWidth( sideBarWidth );
    accountBar->setMinimumWidth( sideBarWidth );
    accountBar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

    //Horizontally lay out 'My accounts' text and 'wallet settings' button side by side.
    {
        QFrame* myAccountsFrame = new QFrame( m_pImpl );
        myAccountsFrame->setObjectName( "frameMyAccounts" );
        QHBoxLayout* layoutMyAccounts = new QHBoxLayout;
        myAccountsFrame->setLayout(layoutMyAccounts);
        myAccountsFrame->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
        accountBar->addWidget( myAccountsFrame );
        myAccountsFrame->setContentsMargins( 0, 0, 0, 0 );
        layoutMyAccounts->setSpacing(0);
        layoutMyAccounts->setContentsMargins( 0, 0, 0, 0 );

        ClickableLabel* myAccountLabel = new ClickableLabel( myAccountsFrame );
        myAccountLabel->setObjectName( "labelMyAccounts" );
        myAccountLabel->setText( tr("My accounts") );
        layoutMyAccounts->addWidget( myAccountLabel );
        myAccountLabel->setContentsMargins( 0, 0, 0, 0 );

        //Spacer to fill width
        {
            QWidget* spacerMid = new QWidget( myAccountsFrame );
            spacerMid->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
            layoutMyAccounts->addWidget( spacerMid );
        }

        ClickableLabel* labelWalletSettings = new ClickableLabel( myAccountsFrame );
        labelWalletSettings->setText( GUIUtil::fontAwesomeRegular("\uf013") );
        labelWalletSettings->setObjectName( "labelWalletSettings" );
        labelWalletSettings->setCursor ( Qt::PointingHandCursor );
        layoutMyAccounts->addWidget( labelWalletSettings );
        labelWalletSettings->setContentsMargins( 0, 0, 0, 0 );

        connect( labelWalletSettings, SIGNAL( clicked() ), this, SLOT( gotoPasswordDialog() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    }

    //Spacer to fill height
    {
        QScrollArea* scrollArea = new QScrollArea ( m_pImpl );
        accountScrollArea = new QFrame( scrollArea );
        scrollArea->setContentsMargins( 0, 0, 0, 0);
        scrollArea->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
        scrollArea->setWidget(accountScrollArea);
        scrollArea->setWidgetResizable(true);

        accountScrollArea->setContentsMargins( 0, 0, 0, 0);
        accountScrollArea->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );


        accountBar->addWidget( scrollArea );

        QVBoxLayout* vbox = new QVBoxLayout();
        vbox->setSpacing(0);
        vbox->setContentsMargins( 0, 0, 0, 0 );

        accountScrollArea->setLayout( vbox );
    }

    ClickableLabel* addAccButton = new ClickableLabel( m_pImpl );
    addAccButton->setText( GUIUtil::fontAwesomeRegular("\uf067 ")+tr("Add account") );
    addAccButton->setObjectName( "add_account_button" );
    addAccButton->setCursor( Qt::PointingHandCursor );
    accountBar->addWidget( addAccButton );
    m_pImpl->addToolBar( Qt::LeftToolBarArea, accountBar );
    connect( addAccButton, SIGNAL( clicked() ), this, SLOT( gotoNewAccountDialog() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );








    //Add the 'Gulden bar' - on the left with the Gulden sign and balance
    guldenBar = new QToolBar( QCoreApplication::translate( "toolbar", "Overview toolbar" ) );
    guldenBar->setFixedHeight( horizontalBarHeight );
    guldenBar->setFixedWidth( sideBarWidth );
    guldenBar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    guldenBar->setMinimumWidth( sideBarWidth );
    guldenBar->setObjectName( "gulden_bar" );
    guldenBar->setMovable( false );
    guldenBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    guldenBar->setIconSize( QSize( 18, 18 ) );


    // We place all the widgets for this action bar inside a frame of fixed width - otherwise the sizing comes out wrong
    {
        balanceContainer = new QFrame();
        QHBoxLayout* layoutBalance = new QHBoxLayout;
        balanceContainer->setLayout(layoutBalance);
        balanceContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
        balanceContainer->setContentsMargins( 0, 0, 0, 0 );
        layoutBalance->setContentsMargins( 0, 0, 0, 0 );
        layoutBalance->setSpacing(0);
        guldenBar->addWidget( balanceContainer );


        //Left margin
        {
            QWidget* spacerL = new QWidget();
            spacerL->setObjectName("gulden_bar_left_margin");
            spacerL->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
            layoutBalance->addWidget( spacerL );
        }

        QLabel* homeIcon = new ClickableLabel( m_pImpl );
        homeIcon->setText("\u0120");
        layoutBalance->addWidget( homeIcon );
        homeIcon->setObjectName( "home_button" );
        homeIcon->setCursor( Qt::PointingHandCursor );
        connect( homeIcon, SIGNAL( clicked() ), this, SLOT( gotoWebsite() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );

        // Use spacer to push balance label to the right
        {
            QWidget* spacerMid = new QWidget();
            spacerMid->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
            layoutBalance->addWidget( spacerMid );
        }

        labelBalance = new ClickableLabel( m_pImpl );
        labelBalance->setObjectName( "gulden_label_balance" );
        labelBalance->setText( "" );
        layoutBalance->addWidget( labelBalance );

        labelBalanceForex = new ClickableLabel( m_pImpl );
        labelBalanceForex->setObjectName( "gulden_label_balance_forex" );
        labelBalanceForex->setText( "" );
        labelBalanceForex->setCursor( Qt::PointingHandCursor );
        connect( labelBalanceForex, SIGNAL( clicked() ), this, SLOT( showExchangeRateDialog() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        labelBalanceForex->setVisible(false);
        layoutBalance->addWidget( labelBalanceForex );

        //Right margin
        {
            QWidget* spacerR = new QWidget();
            spacerR->setObjectName("gulden_bar_right_margin");
            spacerR->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
            layoutBalance->addWidget( spacerR );
        }

        balanceContainer->setMinimumWidth( sideBarWidth );
    }
    m_pImpl->addToolBar( guldenBar );




    //Add spacer bar
    spacerBarL = new QToolBar( QCoreApplication::translate( "toolbar", "Spacer  toolbar" ) );
    spacerBarL->setFixedHeight( horizontalBarHeight );
    spacerBarL->setObjectName( "spacer_bar" );
    spacerBarL->setMovable( false );
    //Spacer to fill width
    {
        QWidget* spacerR = new QWidget();
        spacerR->setMinimumWidth( 40 );
        spacerR->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
        spacerBarL->addWidget( spacerR );
        spacerBarL->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    }
    m_pImpl->addToolBar( spacerBarL );


    tabsBar = m_pImpl->findChildren<QToolBar*>( "" )[0];
    //Add the main toolbar - middle (tabs)
    tabsBar->setFixedHeight( horizontalBarHeight );
    tabsBar->setObjectName( "navigation_bar" );
    tabsBar->setMovable( false );
    tabsBar->setToolButtonStyle( Qt::ToolButtonTextOnly );
    //Remove all the actions so we can add them again in a different order
    tabsBar->removeAction( m_pImpl->historyAction );
    tabsBar->removeAction( m_pImpl->overviewAction );
    tabsBar->removeAction( m_pImpl->sendCoinsAction );
    tabsBar->removeAction( m_pImpl->receiveCoinsAction );
    tabsBar->removeAction( m_pImpl->witnessDialogAction );
    //Setup the tab toolbar
    tabsBar->addAction( m_pImpl->witnessDialogAction );
    tabsBar->addAction( m_pImpl->receiveCoinsAction );
    tabsBar->addAction( m_pImpl->sendCoinsAction );
    tabsBar->addAction( m_pImpl->historyAction );

    passwordAction = new QAction(m_pImpl->platformStyle->SingleColorIcon(":/icons/password"), tr("&Password"), this);
    passwordAction->setStatusTip(tr("Change wallet password"));
    passwordAction->setToolTip(passwordAction->statusTip());
    passwordAction->setCheckable(true);
    passwordAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabsBar->addAction(passwordAction);

    backupAction = new QAction(m_pImpl->platformStyle->SingleColorIcon(":/icons/backup"), tr("&Backup"), this);
    backupAction->setStatusTip(tr("Backup wallet"));
    backupAction->setToolTip(backupAction->statusTip());
    backupAction->setCheckable(true);
    backupAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_6));
    tabsBar->addAction(backupAction);

    connect(passwordAction, SIGNAL(triggered()), this, SLOT(gotoPasswordDialog()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));
    connect(backupAction, SIGNAL(triggered()), this, SLOT(gotoBackupDialog()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));


    m_pImpl->receiveCoinsAction->setChecked( true );

    tabsBar->widgetForAction( m_pImpl->historyAction )->setCursor( Qt::PointingHandCursor );
    tabsBar->widgetForAction( m_pImpl->sendCoinsAction )->setCursor( Qt::PointingHandCursor );
    tabsBar->widgetForAction( m_pImpl->receiveCoinsAction )->setCursor( Qt::PointingHandCursor );
    tabsBar->widgetForAction( m_pImpl->witnessDialogAction )->setCursor( Qt::PointingHandCursor );
    tabsBar->widgetForAction( passwordAction )->setCursor( Qt::PointingHandCursor );
    tabsBar->widgetForAction( backupAction )->setCursor( Qt::PointingHandCursor );
    tabsBar->widgetForAction( m_pImpl->receiveCoinsAction )->setObjectName( "receive_coins_button" );
    tabsBar->widgetForAction( m_pImpl->receiveCoinsAction )->setContentsMargins( 0, 0, 0, 0 );
    tabsBar->widgetForAction( m_pImpl->receiveCoinsAction )->setContentsMargins( 0, 0, 0, 0 );
    tabsBar->widgetForAction( m_pImpl->sendCoinsAction )->setContentsMargins( 0, 0, 0, 0 );
    tabsBar->widgetForAction( m_pImpl->historyAction )->setContentsMargins( 0, 0, 0, 0 );
    tabsBar->widgetForAction( m_pImpl->witnessDialogAction )->setContentsMargins( 0, 0, 0, 0 );
    tabsBar->setContentsMargins( 0, 0, 0, 0 );

    //Spacer to fill width
    {
        QWidget* spacerR = new QWidget();
        spacerR->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
        //Delibritely large amount - to push the next toolbar as far right as possible.
        spacerR->setMinimumWidth( 500000 );
        tabsBar->addWidget( spacerR );
        tabsBar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    }
    //Only show the actions we want
    m_pImpl->receiveCoinsAction->setVisible( true );
    m_pImpl->sendCoinsAction->setVisible( true );
    m_pImpl->historyAction->setVisible( true );
    passwordAction->setVisible( false );
    backupAction->setVisible( false );
    m_pImpl->overviewAction->setVisible( false );
    tabsBar->setWindowTitle( QCoreApplication::translate( "toolbar", "Navigation toolbar" ) );
    m_pImpl->addToolBar( tabsBar );






    //Setup the account info bar
    accountInfoBar = new QToolBar( QCoreApplication::translate( "toolbar", "Account info toolbar" ) );
    accountInfoBar->setFixedHeight( horizontalBarHeight );
    accountInfoBar->setObjectName( "account_info_bar" );
    accountInfoBar->setMovable( false );
    accountInfoBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    m_pImpl->accountSummaryWidget = new AccountSummaryWidget( ticker, m_pImpl );
    m_pImpl->accountSummaryWidget->setObjectName( "settings_button" );
    m_pImpl->accountSummaryWidget->setContentsMargins( 0, 0, 0, 0 );
    accountInfoBar->setContentsMargins( 0, 0, 0, 0 );
    m_pImpl->accountSummaryWidget->setObjectName( "accountSummaryWidget" );
    accountInfoBar->addWidget( m_pImpl->accountSummaryWidget );
    m_pImpl->addToolBar( accountInfoBar );
    connect(m_pImpl->accountSummaryWidget, SIGNAL( requestAccountSettings() ), this, SLOT( showAccountSettings() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect(m_pImpl->accountSummaryWidget, SIGNAL( requestExchangeRateDialog() ), this, SLOT( showExchangeRateDialog() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );


    //Add spacer bar
    spacerBarR = new QToolBar( QCoreApplication::translate( "toolbar", "Spacer  toolbar" ) );
    spacerBarR->setFixedHeight( horizontalBarHeight );
    spacerBarR->setObjectName( "spacer_bar" );
    spacerBarR->setMovable( false );
    //Spacer to fill width
    {
        QWidget* spacerR = new QWidget();
        spacerR->setMinimumWidth( 40 );
        spacerR->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
        spacerBarR->addWidget( spacerR );
        spacerBarR->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    }
    m_pImpl->addToolBar( spacerBarR );

    //Hide all toolbars until UI fully loaded
    hideToolBars();


    //Init the welcome dialog inside walletFrame
    welcomeScreen = new WelcomeDialog(m_pImpl->platformStyle, m_pImpl);
    m_pImpl->walletFrame->walletStack->addWidget(welcomeScreen);
    m_pImpl->walletFrame->walletStack->setCurrentWidget(welcomeScreen);
    connect(welcomeScreen, SIGNAL( loadWallet() ), m_pImpl->walletFrame, SIGNAL( loadWallet() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));
}

void GuldenGUI::hideToolBars()
{
    if(accountBar) accountBar->setVisible(false);
    if(guldenBar) guldenBar->setVisible(false);
    if(spacerBarL) spacerBarL->setVisible(false);
    if(tabsBar) tabsBar->setVisible(false);
    if(spacerBarR) spacerBarR->setVisible(false);
    if(accountInfoBar) accountInfoBar->setVisible(false);
    if(statusBar) statusBar->setVisible(false);
}

void GuldenGUI::showToolBars()
{
    welcomeScreen = NULL;
    m_pImpl->appMenuBar->setStyleSheet("");

    if(accountBar) accountBar->setVisible(true);
    if(guldenBar) guldenBar->setVisible(true);
    if(spacerBarL) spacerBarL->setVisible(true);
    if(tabsBar) tabsBar->setVisible(true);
    if(spacerBarR) spacerBarR->setVisible(true);
    if(accountInfoBar) accountInfoBar->setVisible(true);
    if(statusBar) statusBar->setVisible(m_pImpl->progressBarLabel->isVisible());
}


void GuldenGUI::doApplyStyleSheet()
{
    //Load our own QSS stylesheet template for 'whole app'
    QFile styleFile( ":Gulden/qss" );
    styleFile.open( QFile::ReadOnly );

    //Use the same style on all platforms to simplify skinning
    guldenStyle = new GuldenProxyStyle();
    QApplication::setStyle( guldenStyle );

    if (guldenEventFilter)
    {
        m_pImpl->removeEventFilter(guldenEventFilter);
        delete guldenEventFilter;
    }
    guldenEventFilter = new GuldenEventFilter(m_pImpl->style(), m_pImpl, guldenStyle);
    m_pImpl->installEventFilter(guldenEventFilter);

    //Replace variables in the 'template' with actual values
    QString style( styleFile.readAll() );
    style.replace( "ACCENT_COLOR_1", QString(ACCENT_COLOR_1) );
    style.replace( "ACCENT_COLOR_2", QString(ACCENT_COLOR_2) );
    style.replace( "TEXT_COLOR_1", QString(TEXT_COLOR_1) );
    style.replace( "COLOR_VALIDATION_FAILED", QString(COLOR_VALIDATION_FAILED) );
    
    if (sideBarWidth == sideBarWidthExtended)
    {
        style.replace( "SIDE_BAR_WIDTH", SIDE_BAR_WIDTH_EXTENDED );
    }
    else
    {
        style.replace( "SIDE_BAR_WIDTH", SIDE_BAR_WIDTH );
    }

    style.replace( "LOGO_FONT_SIZE", QString(LOGO_FONT_SIZE) );
    style.replace( "TOOLBAR_FONT_SIZE", QString(TOOLBAR_FONT_SIZE) );
    style.replace( "BUTTON_FONT_SIZE", QString(BUTTON_FONT_SIZE) );
    style.replace( "SMALL_BUTTON_FONT_SIZE", QString(SMALL_BUTTON_FONT_SIZE) );
    style.replace( "MINOR_LABEL_FONT_SIZE", QString(MINOR_LABEL_FONT_SIZE) );

    //Apply the final QSS - after making the 'template substitutions'
    //NB! This should happen last after all object IDs etc. are set.
    m_pImpl->setStyleSheet( style );
}

void GuldenGUI::resizeToolBarsGulden()
{
    //Filler for right of menu bar.
    #ifndef MAC_OSX
    menuBarSpaceFiller->move(sideBarWidth, 0);
    #endif
    accountBar->setFixedWidth( sideBarWidth );
    accountBar->setMinimumWidth( sideBarWidth );
    guldenBar->setFixedWidth( sideBarWidth );
    guldenBar->setMinimumWidth( sideBarWidth ); 
    balanceContainer->setMinimumWidth( sideBarWidth );
}

void GuldenGUI::doPostInit()
{
    //Fonts
    // We 'abuse' the translation system here to allow different 'font stacks' for different languages.
    //QString MAIN_FONTSTACK = QObject::tr("Arial, 'Helvetica Neue', Helvetica, sans-serif");

    m_pImpl->appMenuBar->setStyleSheet("QMenuBar{background-color: rgba(255, 255, 255, 0%);} QMenu{background-color: #f3f4f6; border: 1px solid #999; color: black;} QMenu::item { color: black; } QMenu::item:disabled {color: #999;} QMenu::separator{background-color: #999; height: 1px; margin-left: 10px; margin-right: 5px;}");


    {
        // Qt status bar sucks - it is impossible to style nicely, so we just rip the thing out and use a toolbar instead.
        m_pImpl->statusBar()->setVisible(false);

        //Allow us to target the progress label for easy styling
        //m_pImpl->statusBar()->removeWidget(m_pImpl->progressBarLabel);
        //m_pImpl->statusBar()->removeWidget(m_pImpl->progressBar);
        //m_pImpl->statusBar()->removeWidget(m_pImpl->frameBlocks);

        //Add a spacer to the frameBlocks so that we can force them to expand to same size as the progress text (Needed for proper centering of progress bar)
        /*QFrame* frameBlocksSpacerL = new QFrame(m_pImpl->frameBlocks);
        frameBlocksSpacerL->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        frameBlocksSpacerL->setContentsMargins( 0, 0, 0, 0);
        ((QHBoxLayout*)m_pImpl->frameBlocks->layout())->insertWidget(0, frameBlocksSpacerL);*/

        m_pImpl->frameBlocks->layout()->setContentsMargins( 0, 0, 0, 0 );
        m_pImpl->frameBlocks->layout()->setSpacing( 0 );

        //Hide some of the 'task items' we don't need
        m_pImpl->unitDisplayControl->setVisible( false );
        //m_pImpl->labelBlocksIcon->setVisible( false );

        //Status bar
        {
            statusBar = new QToolBar( QCoreApplication::translate( "toolbar", "Status toolbar" ) , m_pImpl);
            statusBar->setObjectName( "status_bar" );
            statusBar->setMovable( false );

            QFrame* statusBarStatusArea = new QFrame(statusBar);
            statusBarStatusArea->setObjectName("status_bar_status_area");
            statusBarStatusArea->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
            statusBarStatusArea->setContentsMargins( 0, 0, 0, 0);
            QHBoxLayout* statusBarStatusAreaLayout = new QHBoxLayout();
            statusBarStatusArea->setLayout(statusBarStatusAreaLayout);
            statusBarStatusAreaLayout->setSpacing(0);
            statusBarStatusAreaLayout->setContentsMargins( 0, 0, 0, 0 );
            statusBar->addWidget(statusBarStatusArea);

            m_pImpl->progressBarLabel->setObjectName("progress_bar_label");
            statusBarStatusAreaLayout->addWidget(m_pImpl->progressBarLabel);

            QFrame* statusProgressSpacerL = new QFrame(statusBar);
            statusProgressSpacerL->setObjectName("progress_bar_spacer_left");
            statusProgressSpacerL->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
            statusProgressSpacerL->setContentsMargins( 0, 0, 0, 0);
            statusBar->addWidget(statusProgressSpacerL);

            QFrame* progressBarWrapper = new QFrame(statusBar);
            progressBarWrapper->setObjectName("progress_bar");
            progressBarWrapper->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
            progressBarWrapper->setContentsMargins( 0, 0, 0, 0);
            QHBoxLayout* layoutProgressBarWrapper = new QHBoxLayout;
            progressBarWrapper->setLayout(layoutProgressBarWrapper);
            layoutProgressBarWrapper->addWidget(m_pImpl->progressBar);
            statusBar->addWidget(progressBarWrapper);
            m_pImpl->progressBar->setVisible(false);

            QFrame* statusProgressSpacerR = new QFrame(statusBar);
            statusProgressSpacerR->setObjectName("progress_bar_spacer_right");
            statusProgressSpacerR->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
            statusProgressSpacerR->setContentsMargins( 0, 0, 0, 0);
            statusBar->addWidget(statusProgressSpacerR);

            m_pImpl->frameBlocks->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
            m_pImpl->frameBlocks->setObjectName("status_bar_frame_blocks");
            //Use spacer to push all the icons to the right
            QFrame* frameBlocksSpacerL = new QFrame(m_pImpl->frameBlocks);
            frameBlocksSpacerL->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
            frameBlocksSpacerL->setContentsMargins( 0, 0, 0, 0);
            ((QHBoxLayout*)m_pImpl->frameBlocks->layout())->insertWidget(0, frameBlocksSpacerL, 1);
            statusBar->addWidget(m_pImpl->frameBlocks);
            //Right margin to match rest of UI
            QFrame* frameBlocksSpacerR = new QFrame(m_pImpl->frameBlocks);
            frameBlocksSpacerR->setObjectName("rightMargin");
            frameBlocksSpacerR->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
            frameBlocksSpacerR->setContentsMargins( 0, 0, 0, 0);
            ((QHBoxLayout*)m_pImpl->frameBlocks->layout())->addWidget(frameBlocksSpacerR);

            //Use our own styling - clear the styling that is already applied
            m_pImpl->progressBar->setStyleSheet("");
            //Hide text we don't want it as it looks cluttered.
            m_pImpl->progressBar->setTextVisible(false);
            m_pImpl->progressBar->setCursor(Qt::PointingHandCursor);

            m_pImpl->addToolBar( Qt::BottomToolBarArea, statusBar );

            statusBar->setVisible(false);
        }
    }

    m_pImpl->backupWalletAction->setIconText( QCoreApplication::translate( "toolbar", "Backup" ) );

    //Change shortcut keys because we have hidden overview pane and changed tab orders
    m_pImpl->overviewAction->setShortcut( QKeySequence( Qt::ALT + Qt::Key_0 ) );
    m_pImpl->sendCoinsAction->setShortcut( QKeySequence( Qt::ALT + Qt::Key_2 ) );
    m_pImpl->receiveCoinsAction->setShortcut( QKeySequence( Qt::ALT + Qt::Key_3 ) );
    m_pImpl->historyAction->setShortcut( QKeySequence( Qt::ALT + Qt::Key_1 ) );

    doApplyStyleSheet();

    //Force font antialiasing
    QFont f = QApplication::font();
    f.setStyleStrategy( QFont::PreferAntialias );
    QApplication::setFont( f );

    m_pImpl->openAction->setVisible(false);

    m_pImpl->setContextMenuPolicy(Qt::NoContextMenu);

    m_pImpl->setMinimumSize(860, 520);

    disconnect(m_pImpl->backupWalletAction, SIGNAL(triggered()), 0, 0);
    connect(m_pImpl->backupWalletAction, SIGNAL(triggered()), this, SLOT(gotoBackupDialog()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));

    m_pImpl->encryptWalletAction->setCheckable(false);
    disconnect(m_pImpl->encryptWalletAction, SIGNAL(triggered()), 0, 0);
    connect(m_pImpl->encryptWalletAction, SIGNAL(triggered()), this, SLOT(gotoPasswordDialog()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));

    disconnect(m_pImpl->changePassphraseAction, SIGNAL(triggered()), 0, 0);
    connect(m_pImpl->changePassphraseAction, SIGNAL(triggered()), this, SLOT(gotoPasswordDialog()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection));

    labelBalance->setVisible(false);
}

void GuldenGUI::hideProgressBarLabel()
{
    m_pImpl->progressBarLabel->setText("");
    m_pImpl->progressBarLabel->setVisible(false);
    if(statusBar)
        statusBar->setVisible(false);
}

void GuldenGUI::showProgressBarLabel()
{
    m_pImpl->progressBarLabel->setVisible(true);
    if(statusBar)
        statusBar->setVisible(true);
}

void GuldenGUI::hideBalances()
{
    labelBalance->setVisible(false);
    labelBalanceForex->setVisible(false);
    m_pImpl->accountSummaryWidget->hideBalances();
}

void GuldenGUI::showBalances()
{
    if (!labelBalance->isVisible())
    {
        labelBalance->setVisible(true);
        // Give forex label a chance to update if appropriate.
        updateExchangeRates();
        m_pImpl->accountSummaryWidget->showBalances();
    }
}

bool GuldenGUI::welcomeScreenIsVisible()
{
    return welcomeScreen != NULL;
}

QDialog* GuldenGUI::createDialog(QWidget* parent, QString message, QString confirmLabel, QString cancelLabel, int minWidth, int minHeight)
{
    QDialog* d = new QDialog(parent);
    d->setWindowFlags(Qt::Dialog);
    d->setMinimumSize(QSize(minWidth, minHeight));
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->setSpacing(0);
    vbox->setContentsMargins( 0, 0, 0, 0 );

    QLabel* labelDialogMessage = new QLabel(d);
    labelDialogMessage->setText(message);
    labelDialogMessage->setObjectName("labelDialogMessage");
    labelDialogMessage->setContentsMargins( 0, 0, 0, 0 );
    labelDialogMessage->setIndent(0);
    labelDialogMessage->setWordWrap(true);
    vbox->addWidget(labelDialogMessage);

    QWidget* spacer = new QWidget(d);
    spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    vbox->addWidget(spacer);

    QFrame* horizontalLine = new QFrame(d);
    horizontalLine->setFrameStyle(QFrame::HLine);
    horizontalLine->setFixedHeight(1);
    horizontalLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    horizontalLine->setStyleSheet(GULDEN_DIALOG_HLINE_STYLE);
    vbox->addWidget(horizontalLine);

    QDialogButtonBox::StandardButtons buttons;
    if (!confirmLabel.isEmpty())
    {
        buttons |= QDialogButtonBox::Ok;
    }
    if (!cancelLabel.isEmpty())
    {
        // We use reset button because it shows on the left where we want it.
        buttons |= QDialogButtonBox::Reset;
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox(buttons, d);
    vbox->addWidget(buttonBox);
    buttonBox->setContentsMargins( 0, 0, 0, 0 );

    if(!confirmLabel.isEmpty())
    {
        buttonBox->button(QDialogButtonBox::Ok)->setText(confirmLabel);
        buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
        buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(GULDEN_DIALOG_CONFIRM_BUTTON_STYLE);
        QObject::connect(buttonBox, SIGNAL(accepted()), d, SLOT(accept()));
    }

    if (!cancelLabel.isEmpty())
    {
        buttonBox->button(QDialogButtonBox::Reset)->setText(cancelLabel);
        buttonBox->button(QDialogButtonBox::Reset)->setCursor(Qt::PointingHandCursor);
        buttonBox->button(QDialogButtonBox::Reset)->setStyleSheet(GULDEN_DIALOG_CANCEL_BUTTON_STYLE);
        QObject::connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), d, SLOT(reject()));
    }

    d->setLayout(vbox);

    return d;
}

const QString ELLIPSIS("\u2026");
QString limitString(const QString& string, int maxLength)
{
    if (string.length() <= maxLength)
        return string;

    float spacePerPart = (maxLength - ELLIPSIS.length()) / 2.0;
    auto beforeEllipsis = string.left(std::ceil(spacePerPart));
    auto afterEllipsis = string.right(std::floor(spacePerPart));

    return beforeEllipsis + ELLIPSIS + afterEllipsis;
}

QString superscriptSpan(const QString& sText)
{
    return QString("<span style='font-size: 8px;'>%1</span>").arg(sText);
}

QString colourSpan(QString sColour, const QString& sText)
{
    return QString("<span style='color: %1;'>%2</span>").arg(sColour).arg(sText);
}

QString getAccountLabel(CAccount* account)
{
    QString accountName = QString::fromStdString( account->getLabel() );
    accountName = limitString(accountName, 26);

    QString accountNamePrefix;
    if ( account->IsMobi() )
    {
        accountNamePrefix = "\uf10b";
    }
    else if ( account->IsPoW2Witness() )
    {
        if (account->GetWarningState() == AccountStatus::WitnessEmpty)
            accountNamePrefix = GUIUtil::fontAwesomeSolid("\uf19c");
        else if (account->GetWarningState() == AccountStatus::WitnessPending)
            accountNamePrefix = QString("<table cellspacing=0 padding=0><tr><td>%1</td><td valign=top>%2</td><table>").arg(GUIUtil::fontAwesomeRegular("\uf19c")).arg(superscriptSpan(GUIUtil::fontAwesomeSolid("\uf251")));
        else if (account->GetWarningState() == AccountStatus::WitnessExpired)
            accountNamePrefix = QString("<table cellspacing=0 padding=0><tr><td>%1</td><td valign=top>%2</td><table>").arg(GUIUtil::fontAwesomeSolid("\uf19c")).arg(colourSpan("#c97676", superscriptSpan(GUIUtil::fontAwesomeSolid("\uf12a"))));
        else if (account->GetWarningState() == AccountStatus::WitnessEnded)
            accountNamePrefix = QString("<table cellspacing=0 padding=0><tr><td>%1</td><td valign=top>%2</td><table>").arg(GUIUtil::fontAwesomeSolid("\uf19c")).arg(superscriptSpan(GUIUtil::fontAwesomeSolid("\uf11e")));
        else
            accountNamePrefix = QString("<table cellspacing=0 padding=0><tr><td>%1</td><td valign=top>%2</td><table>").arg(GUIUtil::fontAwesomeSolid("\uf19c")).arg(superscriptSpan(GUIUtil::fontAwesomeSolid("\uf023")));
    }
    else if ( !account->IsHD() )
    {
        accountNamePrefix = "\uf187";
    }
    else
    {
        accountNamePrefix = "\uf09d";
    }
    if ( account->IsReadOnly() )
    {
        //fixme: make small if existing prefix
        accountNamePrefix += "\uf06e";
    }
    accountName = QString("<table cellspacing=0 padding=0><tr><td width=10></td><td width=28 align=left>%1</td><td width=2></td><td>%2</td></tr></table>").arg(accountNamePrefix).arg(accountName);

    return accountName;
}

void GuldenGUI::refreshTabVisibilities()
{
    m_pImpl->receiveCoinsAction->setVisible( true );
    m_pImpl->sendCoinsAction->setVisible( true );

    //Required here because when we open wallet and it is already on a read only account restoreCachedWidgetIfNeeded is not called.
    if (pactiveWallet->getActiveAccount()->IsReadOnly())
        m_pImpl->sendCoinsAction->setVisible( false );

    if (pactiveWallet->getActiveAccount()->IsPoW2Witness())
    {
        m_pImpl->receiveCoinsAction->setVisible( false );
        m_pImpl->sendCoinsAction->setVisible( false );
        m_pImpl->witnessDialogAction->setVisible( true );
        if ( m_pImpl->walletFrame->currentWalletView()->currentWidget() == (QWidget*)m_pImpl->walletFrame->currentWalletView()->receiveCoinsPage || m_pImpl->walletFrame->currentWalletView()->currentWidget() == (QWidget*)m_pImpl->walletFrame->currentWalletView()->sendCoinsPage )
        {
            m_pImpl->showWitnessDialog();
        }
    }
    else
    {
        m_pImpl->witnessDialogAction->setVisible( false );
    }
}

QCollator collateAccountsNumerically;
auto cmpAccounts = [&](const QString& s1, const QString& s2){ return collateAccountsNumerically.compare(s1, s2) < 0; };
std::map<QString, CAccount*, decltype(cmpAccounts)> getSortedAccounts()
{
    collateAccountsNumerically.setNumericMode(true);
    std::map<QString, CAccount*, decltype(cmpAccounts)> sortedAccounts(cmpAccounts);
    {
        for ( const auto& accountPair : pactiveWallet->mapAccounts )
        {
            if (accountPair.second->m_State == AccountState::Normal || (fShowChildAccountsSeperately && accountPair.second->m_State == AccountState::ShadowChild) )
                sortedAccounts[QString::fromStdString(accountPair.second->getLabel())] = accountPair.second;
        }
    }
    return sortedAccounts;
}

void GuldenGUI::refreshAccountControls()
{
    LogPrintf("GuldenGUI::refreshAccountControls\n");

    refreshTabVisibilities();

    if (pactiveWallet)
    {
        //Disable layout to prevent updating to changes immediately
        accountScrollArea->layout()->setEnabled(false);
        {
            // Get an ordered list of old account labels.
            std::vector<ClickableLabel*> allLabels;
            for (unsigned int i=0; i<accountScrollArea->layout()->count(); ++i)
            {
                allLabels.push_back(dynamic_cast<ClickableLabel*>((accountScrollArea->layout()->itemAt(i)->widget())));
            }
            m_accountMap.clear();

            // Sort the accounts.
            ClickableLabel* makeActive = NULL;
            std::map<QString, CAccount*, decltype(cmpAccounts)> sortedAccounts = getSortedAccounts();
            {
                //NB! Mutex scope here is important to avoid deadlock inside setActiveAccountButton
                LOCK(pactiveWallet->cs_wallet);

                // Update to the sorted list
                int nCount = 0;
                for (const auto& sortedIter : sortedAccounts)
                {
                    ClickableLabel* accLabel = nullptr;
                    QString label = getAccountLabel(sortedIter.second);
                    if (allLabels.size() >= nCount+1)
                    {
                        accLabel = allLabels[nCount];
                        accLabel->setText( label );
                    }
                    else
                    {
                        accLabel = createAccountButton( label );
                        accountScrollArea->layout()->addWidget( accLabel );
                    }
                    m_accountMap[accLabel] = sortedIter.second;
                    if (sortedIter.second->getUUID() == m_pImpl->walletFrame->currentWalletView()->walletModel->getActiveAccount()->getUUID())
                        makeActive = accLabel;
                    ++nCount;
                }

                // Remove any excess widgets that are still in UI if required.
                for (;nCount < allLabels.size();++nCount)
                {
                    accountScrollArea->layout()->removeWidget(allLabels[nCount]);
                }
            }
            if (makeActive)
            setActiveAccountButton( makeActive );
        }
        // Force layout to update now that all the changes are made.
        accountScrollArea->layout()->setEnabled(true);
    }
}

bool GuldenGUI::setCurrentWallet( const QString& name )
{
    LogPrintf("GuldenGUI::setCurrentWallet %s\n", name.toStdString());

    showToolBars();
    refreshAccountControls();

    connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount) ), m_pImpl->accountSummaryWidget , SLOT( balanceChanged() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount) ), this , SLOT( balanceChanged() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( accountListChanged() ), this , SLOT( accountListChanged() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( activeAccountChanged(CAccount*) ), this , SLOT( activeAccountChanged(CAccount*) ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( accountDeleted(CAccount*) ), this , SLOT( accountDeleted(CAccount*) ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( accountAdded(CAccount*) ), this , SLOT( accountAdded(CAccount*) ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->witnessDialogPage, SIGNAL(requestEmptyWitness()), this, SLOT(requestEmptyWitness()), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->witnessDialogPage, SIGNAL(requestFundWitness(CAccount*)), this, SLOT(requestFundWitness(CAccount*)), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    connect( m_pImpl->walletFrame->currentWalletView()->witnessDialogPage, SIGNAL(requestRenewWitness(CAccount*)), this, SLOT(requestRenewWitness(CAccount*)) );
    connect( m_pImpl->walletFrame->currentWalletView()->sendCoinsPage, SIGNAL(notifyPaymentAccepted()), this, SLOT(handlePaymentAccepted()) );

    return true;
}


ClickableLabel* GuldenGUI::createAccountButton( const QString& accountName )
{
    ClickableLabel* newAccountButton = new ClickableLabel( m_pImpl );
    newAccountButton->setTextFormat( Qt::RichText );
    newAccountButton->setText( accountName );
    newAccountButton->setCursor( Qt::PointingHandCursor );
    m_pImpl->connect( newAccountButton, SIGNAL( clicked() ), this, SLOT( accountButtonPressed() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
    return newAccountButton;
}

void GuldenGUI::setActiveAccountButton( ClickableLabel* activeButton )
{
    LogPrintf("GuldenGUI::setActiveAccountButton\n");

    for ( const auto & button : accountBar->findChildren<ClickableLabel*>( "" ) )
    {
        button->setChecked( false );
        button->setCursor( Qt::PointingHandCursor );
    }
    activeButton->setChecked( true );
    activeButton->setCursor( Qt::ArrowCursor );

     // Update the account
    if ( m_pImpl->walletFrame->currentWalletView() )
    {
        if ( m_pImpl->walletFrame->currentWalletView()->receiveCoinsPage )
        {
            m_pImpl->accountSummaryWidget->setActiveAccount( m_accountMap[activeButton] );
            m_pImpl->walletFrame->currentWalletView()->walletModel->setActiveAccount( m_accountMap[activeButton] );

            updateAccount(m_accountMap[activeButton]);
        }
    }
}

void GuldenGUI::updateAccount(CAccount* account)
{
    LOCK(pactiveWallet->cs_wallet);

    if (receiveAddress)
        delete receiveAddress;

    receiveAddress = new CReserveKey(pactiveWallet, account, KEYCHAIN_EXTERNAL);
    CPubKey pubKey;
    if (receiveAddress->GetReservedKey(pubKey))
    {
        CKeyID keyID = pubKey.GetID();
        m_pImpl->walletFrame->currentWalletView()->receiveCoinsPage->updateAddress( QString::fromStdString(CGuldenAddress(keyID).ToString()) );
    }
    else
    {
        m_pImpl->walletFrame->currentWalletView()->receiveCoinsPage->updateAddress( "error" );
    }
    m_pImpl->walletFrame->currentWalletView()->receiveCoinsPage->setActiveAccount( account );
}

void GuldenGUI::balanceChanged()
{
    // Force receive Qr code to update on balance change.
    if (m_pImpl && m_pImpl->walletFrame && m_pImpl->walletFrame->currentWalletView() && m_pImpl->walletFrame->currentWalletView()->walletModel)
        updateAccount( m_pImpl->walletFrame->currentWalletView()->walletModel->getActiveAccount() );
}


void GuldenGUI::accountListChanged()
{
    refreshAccountControls();
}

void GuldenGUI::activeAccountChanged(CAccount* account)
{
    if (m_pImpl->accountSummaryWidget)
        m_pImpl->accountSummaryWidget->setActiveAccount(account);

    refreshTabVisibilities();

    //Update account name 'in place' in account list
    bool haveAccount=false;
    if (pactiveWallet)
    {
        for ( const auto& accountPair : m_accountMap )
        {
            if (accountPair.second == account)
            {
                haveAccount = true;
                if (!accountPair.first->isChecked())
                {
                    setActiveAccountButton(accountPair.first);
                }
                accountPair.first->setTextFormat( Qt::RichText );
                accountPair.first->setText( getAccountLabel(account) );
            }
        }
    }

    if(!haveAccount)
    {
        refreshAccountControls();
    }
}

// For performance reasons we update only the specific control that is added instead of regenerating all controls.
void GuldenGUI::accountAdded(CAccount* addedAccount)
{
    //Disable layout to prevent updating to changes immediately
    accountScrollArea->layout()->setEnabled(false);
    if (pactiveWallet)
    {
        //NB! Mutex scope here is important to avoid deadlock inside setActiveAccountButton
        LOCK(pactiveWallet->cs_wallet);

        std::map<QString, CAccount*, decltype(cmpAccounts)> sortedAccounts = getSortedAccounts();
        for (const auto& sortedIter : sortedAccounts)
        {
            if (sortedIter.second->getUUID() == addedAccount->getUUID())
            {
                QString label = getAccountLabel(sortedIter.second);
                ClickableLabel* accLabel = createAccountButton( label );
                m_accountMap[accLabel] = sortedIter.second;
                int pos = std::distance(sortedAccounts.begin(), sortedAccounts.find(QString::fromStdString(addedAccount->getLabel())));
                ((QVBoxLayout*)accountScrollArea->layout())->insertWidget(pos, accLabel);
                break;
            }
        }
    }
    // Force layout to update now that all the changes are made.
    accountScrollArea->layout()->setEnabled(true);
}

//fixme: (Post-2.1) - This is a bit lazy; accountAdded/accountDeleted should instead update only the specific controls affected instead of all the controls.
void GuldenGUI::accountDeleted(CAccount* account)
{
    refreshAccountControls();
}

void GuldenGUI::accountButtonPressed()
{
    QObject* sender = this->sender();
    ClickableLabel* accButton = qobject_cast<ClickableLabel*>( sender );
    setActiveAccountButton( accButton );
    restoreCachedWidgetIfNeeded();
}



void GuldenGUI::promptImportPrivKey()
{
    ImportPrivKeyDialog dlg(this->m_pImpl);
    dlg.exec();

    CGuldenSecret vchSecret;
    bool fGood = vchSecret.SetString(dlg.getPrivKey().c_str());

    if (fGood)
    {
        LOCK2(cs_main, pactiveWallet->cs_wallet);

        CKey key = vchSecret.GetKey();
        if (!key.IsValid())
        {
            m_pImpl->message(tr("Error importing private key"), tr("Invalid private key."), CClientUIInterface::MSG_ERROR, NULL);
            return;
        }

        CPubKey pubkey = key.GetPubKey();
        assert(key.VerifyPubKey(pubkey));
        CKeyID vchAddress = pubkey.GetID();

        //Don't import an address that is already in wallet.
        if (pactiveWallet->HaveKey(vchAddress))
        {
            m_pImpl->message(tr("Error importing private key"), tr("Wallet already contains key."), CClientUIInterface::MSG_ERROR, NULL);
            return;
        }

        CAccount* pAccount = pactiveWallet->GenerateNewLegacyAccount(tr("Imported legacy").toStdString());
        pactiveWallet->MarkDirty();
        pactiveWallet->mapKeyMetadata[vchAddress].nCreateTime = 1;

        if (!pactiveWallet->AddKeyPubKey(key, pubkey, *pAccount, KEYCHAIN_EXTERNAL))
        {
            m_pImpl->message(tr("Error importing private key"), tr("Failed to add key to wallet."), CClientUIInterface::MSG_ERROR, NULL);
            return;
        }

        // Whenever a key is imported, we need to scan the whole chain - do so now
        pactiveWallet->nTimeFirstKey = 1;
        boost::thread t(rescanThread); // thread runs free
    }
}

void GuldenGUI::promptRescan()
{
    // Whenever a key is imported, we need to scan the whole chain - do so now
    pactiveWallet->nTimeFirstKey = 1;
    boost::thread t(rescanThread); // thread runs free
}

void GuldenGUI::gotoWebsite()
{
    QDesktopServices::openUrl( QUrl( "http://www.Gulden.com/" ) );
}

void GuldenGUI::restoreCachedWidgetIfNeeded()
{
    bool stateReceiveCoinsAction = true;
    bool stateSendCoinsAction = true;

    m_pImpl->walletFrame->currentWalletView()->sendCoinsPage->update();
    m_pImpl->walletFrame->currentWalletView()->witnessDialogPage->update();

    if (pactiveWallet->getActiveAccount()->IsReadOnly())
    {
        stateSendCoinsAction = false;
        if ( m_pImpl->walletFrame->currentWalletView()->currentWidget() == (QWidget*)m_pImpl->walletFrame->currentWalletView()->sendCoinsPage )
        {
            m_pImpl->gotoReceiveCoinsPage();
        }
    }
    if (pactiveWallet->getActiveAccount()->IsPoW2Witness())
    {
        m_pImpl->witnessDialogAction->setVisible( true );
        stateReceiveCoinsAction = false;
        stateSendCoinsAction = false;
        if ( m_pImpl->walletFrame->currentWalletView()->currentWidget() == (QWidget*)m_pImpl->walletFrame->currentWalletView()->receiveCoinsPage || m_pImpl->walletFrame->currentWalletView()->currentWidget() == (QWidget*)m_pImpl->walletFrame->currentWalletView()->sendCoinsPage )
        {
            m_pImpl->showWitnessDialog();
        }
    }
    else
    {
        m_pImpl->witnessDialogAction->setVisible( false );
        if ( m_pImpl->walletFrame->currentWalletView()->currentWidget() == (QWidget*)m_pImpl->walletFrame->currentWalletView()->witnessDialogPage )
        {
            m_pImpl->gotoReceiveCoinsPage();
        }
    }

    m_pImpl->historyAction->setVisible( true );
    passwordAction->setVisible( false );
    backupAction->setVisible( false );
    m_pImpl->overviewAction->setVisible( true );

    if (dialogPasswordModify)
    {
        m_pImpl->walletFrame->currentWalletView()->removeWidget( dialogPasswordModify );
        dialogPasswordModify->deleteLater();
        dialogPasswordModify = NULL;
    }
    if (dialogBackup)
    {
        m_pImpl->walletFrame->currentWalletView()->removeWidget( dialogBackup );
        dialogBackup->deleteLater();
        dialogBackup = NULL;
    }
    if (dialogNewAccount)
    {
        m_pImpl->walletFrame->currentWalletView()->removeWidget( dialogNewAccount );
        dialogNewAccount->deleteLater();
        dialogNewAccount = NULL;
    }
    if (dialogAccountSettings)
    {
        m_pImpl->walletFrame->currentWalletView()->removeWidget( dialogAccountSettings );
        dialogAccountSettings->deleteLater();
        dialogAccountSettings = NULL;
    }
    if (cacheCurrentWidget)
    {
        m_pImpl->walletFrame->currentWalletView()->setCurrentWidget( cacheCurrentWidget );
        cacheCurrentWidget = NULL;
    }

    m_pImpl->receiveCoinsAction->setVisible( stateReceiveCoinsAction );
    m_pImpl->sendCoinsAction->setVisible( stateSendCoinsAction );
}

void GuldenGUI::gotoNewAccountDialog()
{
    if ( m_pImpl->walletFrame )
    {
        restoreCachedWidgetIfNeeded();

        dialogNewAccount = new NewAccountDialog( m_pImpl->platformStyle, m_pImpl->walletFrame->currentWalletView(), m_pImpl->walletFrame->currentWalletView()->walletModel);
        connect( dialogNewAccount, SIGNAL( cancel() ), this, SLOT( cancelNewAccountDialog() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        connect( dialogNewAccount, SIGNAL( accountAdded() ), this, SLOT( acceptNewAccount() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        connect( dialogNewAccount, SIGNAL( addAccountMobile() ), this, SLOT( acceptNewAccountMobile() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        cacheCurrentWidget = m_pImpl->walletFrame->currentWalletView()->currentWidget();
        m_pImpl->walletFrame->currentWalletView()->addWidget( dialogNewAccount );
        m_pImpl->walletFrame->currentWalletView()->setCurrentWidget( dialogNewAccount );
    }
}

void GuldenGUI::gotoPasswordDialog()
{
    if ( m_pImpl->walletFrame )
    {
        restoreCachedWidgetIfNeeded();

        passwordAction->setVisible( true );
        backupAction->setVisible( true );
        passwordAction->setChecked(true);
        backupAction->setChecked(false);

        m_pImpl->receiveCoinsAction->setVisible( false );
        m_pImpl->sendCoinsAction->setVisible( false );
        m_pImpl->historyAction->setVisible( false );
        m_pImpl->overviewAction->setVisible( false );
        m_pImpl->witnessDialogAction->setVisible( false );

        dialogPasswordModify = new PasswordModifyDialog( m_pImpl->platformStyle, m_pImpl->walletFrame->currentWalletView() );
        connect( dialogPasswordModify, SIGNAL( dismiss() ), this, SLOT( dismissPasswordDialog() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        cacheCurrentWidget = m_pImpl->walletFrame->currentWalletView()->currentWidget();
        m_pImpl->walletFrame->currentWalletView()->addWidget( dialogPasswordModify );
        m_pImpl->walletFrame->currentWalletView()->setCurrentWidget( dialogPasswordModify );
    }
}

void GuldenGUI::gotoBackupDialog()
{
    if ( m_pImpl->walletFrame )
    {
        restoreCachedWidgetIfNeeded();

        passwordAction->setVisible( true );
        backupAction->setVisible( true );
        passwordAction->setChecked(false);
        backupAction->setChecked(true);

        m_pImpl->receiveCoinsAction->setVisible( false );
        m_pImpl->sendCoinsAction->setVisible( false );
        m_pImpl->historyAction->setVisible( false );
        m_pImpl->overviewAction->setVisible( false );
        m_pImpl->witnessDialogAction->setVisible( false );

        dialogBackup = new BackupDialog( m_pImpl->platformStyle, m_pImpl->walletFrame->currentWalletView(), m_pImpl->walletFrame->currentWalletView()->walletModel);
        connect( dialogBackup, SIGNAL( saveBackupFile() ), m_pImpl->walletFrame, SLOT( backupWallet() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        connect( dialogBackup, SIGNAL( dismiss() ), this, SLOT( dismissBackupDialog() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        cacheCurrentWidget = m_pImpl->walletFrame->currentWalletView()->currentWidget();
        m_pImpl->walletFrame->currentWalletView()->addWidget( dialogBackup );
        m_pImpl->walletFrame->currentWalletView()->setCurrentWidget( dialogBackup );
    }
}

void GuldenGUI::dismissBackupDialog()
{
    restoreCachedWidgetIfNeeded();
}

void GuldenGUI::dismissPasswordDialog()
{
    restoreCachedWidgetIfNeeded();
}



void GuldenGUI::cancelNewAccountDialog()
{
    restoreCachedWidgetIfNeeded();
}

void GuldenGUI::acceptNewAccount()
{ 
    if ( !dialogNewAccount->getAccountName().simplified().isEmpty() )
    {
        CAccount* newAccount = nullptr;
        const auto newAccountType = dialogNewAccount->getAccountType();
        if (newAccountType == NewAccountType::FixedDeposit)
        {
            newAccount = pactiveWallet->GenerateNewAccount(dialogNewAccount->getAccountName().toStdString(), AccountState::Normal, AccountType::PoW2Witness);
        }
        else
        {
            newAccount = pactiveWallet->GenerateNewAccount(dialogNewAccount->getAccountName().toStdString(), AccountState::Normal, AccountType::Desktop);
        }

        if (!newAccount)
        {
            // Temporarily unlock for account generation.
            std::function<void (void)> successCallback = [&](){this->acceptNewAccount(); pactiveWallet->Lock();};
            uiInterface.RequestUnlockWithCallback(pactiveWallet, _("Wallet unlock required for account creation"), successCallback);
            return;
        }
        restoreCachedWidgetIfNeeded();
        if (newAccountType == NewAccountType::FixedDeposit)
        {
            newAccount->SetWarningState(AccountStatus::WitnessEmpty);
            static_cast<const CGuldenWallet*>(pactiveWallet)->NotifyAccountWarningChanged(pactiveWallet, newAccount);
            m_pImpl->showWitnessDialog();
        }
        else
        {
            m_pImpl->gotoReceiveCoinsPage();
        }
    }
    else
    {
        //fixme: (2.1) Mark invalid.
    }
}

void GuldenGUI::acceptNewAccountMobile()
{
    restoreCachedWidgetIfNeeded();
}

void GuldenGUI::showAccountSettings()
{
    LogPrintf("GuldenGUI::showAccountSettings\n");

    if ( m_pImpl->walletFrame )
    {
        restoreCachedWidgetIfNeeded();

        dialogAccountSettings = new AccountSettingsDialog( m_pImpl->platformStyle, m_pImpl->walletFrame->currentWalletView(), m_pImpl->walletFrame->currentWalletView()->walletModel->getActiveAccount(), m_pImpl->walletFrame->currentWalletView()->walletModel);
        connect( dialogAccountSettings, SIGNAL( dismissAccountSettings() ), this, SLOT( dismissAccountSettings() ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( activeAccountChanged(CAccount*) ), dialogAccountSettings, SLOT( activeAccountChanged(CAccount*) ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        cacheCurrentWidget = m_pImpl->walletFrame->currentWalletView()->currentWidget();
        m_pImpl->walletFrame->currentWalletView()->addWidget( dialogAccountSettings );
        m_pImpl->walletFrame->currentWalletView()->setCurrentWidget( dialogAccountSettings );
    }
}

void GuldenGUI::dismissAccountSettings()
{
    restoreCachedWidgetIfNeeded();
}

void GuldenGUI::showExchangeRateDialog()
{
    if (!dialogExchangeRate)
    {
        CurrencyTableModel* currencyTabelmodel = ticker->GetCurrencyTableModel();
        currencyTabelmodel->setBalance( m_pImpl->walletFrame->currentWalletView()->walletModel->getBalance() );
        connect( m_pImpl->walletFrame->currentWalletView()->walletModel, SIGNAL( balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount) ), currencyTabelmodel , SLOT( balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount) ), (Qt::ConnectionType)(Qt::AutoConnection|Qt::UniqueConnection) );
        dialogExchangeRate = new ExchangeRateDialog( m_pImpl->platformStyle, m_pImpl, currencyTabelmodel );
        dialogExchangeRate->setOptionsModel( optionsModel );
    }
    dialogExchangeRate->show();
}





std::string CurrencySymbolForCurrencyCode(const std::string& currencyCode)
{
    static std::map<std::string, std::string> currencyCodeSymbolMap = {
        {"ALL", "Lek"},
        {"AFN", "؋"},
        {"ARS", "$"},
        {"AWG", "ƒ"},
        {"AUD", "$"},
        {"AZN", "ман"},
        {"BSD", "$"},
        {"BBD", "$"},
        {"BYN", "Br"},
        {"BZD", "BZ$"},
        {"BMD", "$"},
        {"BOB", "$b"},
        {"BAM", "KM"},
        {"BWP", "P"},
        {"BGN", "лв"},
        {"BRL", "R$"},
        {"BND", "$"},
        {"BTC", "\uF15A"},
        {"KHR", "៛"},
        {"CAD", "$"},
        {"KYD", "$"},
        {"CLP", "$"},
        {"CNY", "¥"},
        {"COP", "$"},
        {"CRC", "₡"},
        {"HRK", "kn"},
        {"CUP", "₱"},
        {"CZK", "Kč"},
        {"DKK", "kr"},
        {"DOP", "RD$"},
        {"XCD", "$"},
        {"EGP", "£"},
        {"SVC", "$"},
        {"EUR", "€"},
        {"FKP", "£"},
        {"FJD", "$"},
        {"GHS", "¢"},
        {"GIP", "£"},
        {"GTQ", "Q"},
        {"GGP", "£"},
        {"GYD", "$"},
        {"HNL", "L"},
        {"HKD", "$"},
        {"HUF", "Ft"},
        {"ISK", "kr"},
        {"INR", ""},
        {"IDR", "Rp"},
        {"IRR", "﷼"},
        {"IMP", "£"},
        {"ILS", "₪"},
        {"JMD", "J$"},
        {"JPY", "¥"},
        {"JEP", "£"},
        {"KZT", "лв"},
        {"KPW", "₩"},
        {"KRW", "₩"},
        {"KGS", "лв"},
        {"LAK", "₭"},
        {"LBP", "£"},
        {"LRD", "$"},
        {"MKD", "ден"},
        {"MYR", "RM"},
        {"MUR", "₨"},
        {"MXN", "$"},
        {"MNT", "₮"},
        {"MZN", "MT"},
        {"NAD", "$"},
        {"NPR", "₨"},
        {"ANG", "ƒ"},
        {"NZD", "$"},
        {"NIO", "C$"},
        {"NGN", "₦"},
        {"KPW", "₩"},
        {"NOK", "kr"},
        {"OMR", "﷼"},
        {"PKR", "₨"},
        {"PAB", "B/."},
        {"PYG", "Gs"},
        {"PEN", "S/."},
        {"PHP", "₱"},
        {"PLN", "zł"},
        {"QAR", "﷼"},
        {"RON", "lei"},
        {"RUB", "₽"},
        {"SHP", "£"},
        {"SAR", "﷼"},
        {"RSD", "Дин."},
        {"SCR", "₨"},
        {"SGD", "$"},
        {"SBD", "$"},
        {"SOS", "S"},
        {"ZAR", "R"},
        {"KRW", "₩"},
        {"LKR", "₨"},
        {"SEK", "kr"},
        {"CHF", "CHF"},
        {"SRD", "$"},
        {"SYP", "£"},
        {"TWD", "NT$"},
        {"THB", "฿"},
        {"TTD", "TT$"},
        {"TRY", ""},
        {"TVD", "$"},
        {"UAH", "₴"},
        {"GBP", "£"},
        {"USD", "$"},
        {"UYU", "$U"},
        {"UZS", "лв"},
        {"VEF", "Bs"},
        {"VND", "₫"},
        {"YER", "﷼"},
        {"ZWD", "Z$"},
        {"NLG", "\u0120"}
    };

    if (currencyCodeSymbolMap.find(currencyCode) != currencyCodeSymbolMap.end())
    {
        return currencyCodeSymbolMap[currencyCode];
    }
    return "";
}

