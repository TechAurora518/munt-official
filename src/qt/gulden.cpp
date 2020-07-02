// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#if defined(HAVE_CONFIG_H)
#include "config/build-config.h"
#endif

#include "gui.h"
#include <unity/appmanager.h>

#include "chainparams.h"
#include "clientmodel.h"
#include "fs.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "intro.h"
#include "networkstyle.h"
#include "optionsmodel.h"
#include "utilitydialog.h"
#include "winshutdownmonitor.h"

#include "net.h"

#ifdef ENABLE_WALLET
#include "paymentserver.h"
#include "walletmodel.h"
#endif

#include "init.h"
#include "rpc/server.h"
#include "scheduler.h"
#include "ui_interface.h"
#include "util.h"
#include "warnings.h"

#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

#include <stdint.h>

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/thread.hpp>

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QThread>
#include <QTimer>
#include <QTranslator>
#include <QSslConfiguration>
#include <QKeyEvent>

#if defined(QT_STATICPLUGIN)
#include <QtPlugin>
#if QT_VERSION < 0x050000
Q_IMPORT_PLUGIN(qcncodecs)
Q_IMPORT_PLUGIN(qjpcodecs)
Q_IMPORT_PLUGIN(qtwcodecs)
Q_IMPORT_PLUGIN(qkrcodecs)
Q_IMPORT_PLUGIN(qtaccessiblewidgets)
#else
#if QT_VERSION < 0x050400
Q_IMPORT_PLUGIN(AccessibleFactory)
#endif
#if defined(QT_QPA_PLATFORM_XCB)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_WINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_COCOA)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
#endif
#endif

#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

//fixme: (FUT) (BUILD_SYSTEM) - Enable turning this on for special debugging cases
//#define LOG_ALL_QT_EVENTS

static void InitMessage(const std::string &message)
{
    LogPrintf("init message: %s\n", message);
}

/*
   Translate string to current locale using Qt.
 */
static std::string Translate(const char* psz)
{
    return QCoreApplication::translate("Gulden", psz).toStdString();
}

static QString GetLangTerritory()
{
    QSettings settings;
    // Get desired locale (e.g. "de_DE")
    // 1) System default language
    QString lang_territory = QLocale::system().name();
    // 2) Language from QSettings
    QString lang_territory_qsettings = settings.value("language", "").toString();
    if(!lang_territory_qsettings.isEmpty())
        lang_territory = lang_territory_qsettings;
    // 3) -lang command line argument
    lang_territory = QString::fromStdString(GetArg("-lang", lang_territory.toStdString()));
    return lang_territory;
}

/** Set up translations */
static void initTranslations(QTranslator &qtTranslatorBase, QTranslator &qtTranslator, QTranslator &translatorBase, QTranslator &translator)
{
    // Remove old translators
    QApplication::removeTranslator(&qtTranslatorBase);
    QApplication::removeTranslator(&qtTranslator);
    QApplication::removeTranslator(&translatorBase);
    QApplication::removeTranslator(&translator);

    // Get desired locale (e.g. "de_DE")
    // 1) System default language
    QString lang_territory = GetLangTerritory();

    // Convert to "de" only by truncating "_DE"
    QString lang = lang_territory;
    lang.truncate(lang_territory.lastIndexOf('_'));

    // Load language files for configured locale:
    // - First load the translator for the base language, without territory
    // - Then load the more specific locale translator

    // Load e.g. qt_de.qm
    if (qtTranslatorBase.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTranslatorBase);

    // Load e.g. qt_de_DE.qm
    if (qtTranslator.load("qt_" + lang_territory, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTranslator);

    // Load e.g. gulden_de.qm (shortcut "de" needs to be defined in Gulden.qrc)
    if (translatorBase.load(lang, ":/translations/"))
        QApplication::installTranslator(&translatorBase);

    // Load e.g. gulden_de_DE.qm (shortcut "de_DE" needs to be defined in Gulden.qrc)
    if (translator.load(lang_territory, ":/translations/"))
        QApplication::installTranslator(&translator);
}

/* qDebug() message handler --> debug.log */
#if QT_VERSION < 0x050000
static void DebugMessageHandler(QtMsgType type, const char *msg)
{
    if (type == QtDebugMsg) {
        LogPrint(BCLog::QT, "GUI: %s\n", msg);
    } else {
        LogPrintf("GUI: %s\n", msg);
    }
}
#else
static void DebugMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString &msg)
{
    (unused)context;
    if (type == QtDebugMsg) {
        LogPrint(BCLog::QT, "GUI: %s\n", msg.toStdString());
    } else {
        LogPrintf("GUI: %s\n", msg.toStdString());
    }
}
#endif

class GuldenProxyStyle : public QProxyStyle
{
    Q_OBJECT
public:
    GuldenProxyStyle();
    void drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
    public:
    void setAltDown(bool altDown_) const
    {
        altDown = altDown_;
    }
    bool isAltDown() const
    {
        return altDown;
    }
private:
    mutable bool altDown;
};

class GuldenEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit GuldenEventFilter(QObject* parent, const GuldenProxyStyle* style_, QWidget* window_)
    : QObject(parent)
    , style(style_)
    , window(window_)
    {
    }
protected:
    bool eventFilter(QObject *obj, QEvent *evt);

private:
    const GuldenProxyStyle* style;
    QWidget* window;
};

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
            style->setAltDown(evt->type() == QEvent::KeyPress);
            window->repaint();
        }
    }
    else if(evt->type() == QEvent::ApplicationDeactivate || evt->type() == QEvent::ApplicationStateChange || evt->type() == QEvent::Leave)
    {
        if(style->isAltDown())
        {
            style->setAltDown(false);
            window->repaint();
        }
    }
    return QObject::eventFilter(obj, evt);
}

#include <QMetaEnum>
/** Main Gulden application object */
class GuldenApplication: public QApplication
{
    Q_OBJECT
public:
    explicit GuldenApplication(int &argc, char **argv);
    virtual ~GuldenApplication();

#ifdef ENABLE_WALLET
    /// Create payment server
    void createPaymentServer();
#endif
    /// parameter interaction/setup based on rules
    void parameterSetup();
    /// Create options model
    void createOptionsModel(bool resetSettings);
    /// Create main window
    void createWindow();
    /// Create the "network style" object to handle styling differences between mainnet/testnet etc.
    void createNetworkStyle(QString networkIDString);

    #ifdef LOG_ALL_QT_EVENTS
    bool logEvents = false;
    virtual bool notify(QObject* receiver, QEvent* event)
    {
        if (logEvents)
        {
            static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
            QString name = QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(event->type());
            if (!name.isEmpty())
            {
                QString receiveName = receiver->objectName().toLocal8Bit().data();
                LogPrintf("QTEVENT: event type %s for object %s\n", name.toStdString().c_str(), receiveName.toStdString().c_str());
                if (name == "ActionRemoved" && receiveName == "navigation_bar")
                {
                    int nBreak = 1;
                }
            }
        }
        QApplication::notify(receiver, event);
    }
    #endif

    /// Get window identifier of QMainWindow (GUI)
    WId getMainWinId() const;

public Q_SLOTS:
    /// Request core initialization
    void requestInitialize();
    void initializeResult(bool success);
    void shutdown_hideUIForShutdown();
    void shutdown_InitialUINotification();
    void shutdown_CloseModels();
    void shutdown_TerminateApp();
    void shutdown_MainWindowDestroyed();
    /// Handle runaway exceptions. Shows a message box with the problem and quits the program.
    void handleRunawayException(const QString &message);

Q_SIGNALS:

private:
    OptionsModel *optionsModel = nullptr;
    ClientModel *clientModel = nullptr;
    GUI *window = nullptr;
#ifdef ENABLE_WALLET
    PaymentServer* paymentServer = nullptr;
    WalletModel *walletModel = nullptr;
#endif
    const GuldenProxyStyle* platformStyle = nullptr;
    NetworkStyle* networkStyle = nullptr;
    QWidget* shutdownWindow = nullptr;

    // GULDEN - rescan code needs this to tell if we try shut down midway through a rescan.
    bool shutDownRequested = false;
};

#include "gulden.moc"

GuldenApplication::GuldenApplication(int &argc, char **argv)
: QApplication(argc, argv)
, platformStyle(new GuldenProxyStyle())
{
#ifdef Q_OS_MAC
    setQuitOnLastWindowClosed(true);
#else
    // Don't automatically close, this will exit the application early without proper shutdown on Windows
    // when "Minimize to tray" is enabled and the Debug windows is closed (https://github.com/Gulden/gulden-official/issues/142)
    setQuitOnLastWindowClosed(false);
#endif

    // Use the same style on all platforms to simplify skinning
    setStyle(dynamic_cast<QStyle*>(const_cast<GuldenProxyStyle*>(platformStyle)));
    assert(platformStyle);

    /*  communication to and from initialisation/shutdown threads */
    THREADSAFE_CONNECT_UI_SIGNAL_TO_CORE_SIGNAL1(AppLifecycleManager::gApp->signalRunawayException, std::string, this, "handleRunawayException");
    THREADSAFE_CONNECT_UI_SIGNAL_TO_CORE_SIGNAL1(AppLifecycleManager::gApp->signalAppInitializeResult, bool, this, "initializeResult");
    THREADSAFE_CONNECT_UI_SIGNAL_TO_CORE_SIGNAL0(AppLifecycleManager::gApp->signalAppShutdownStarted, this, "shutdown_hideUIForShutdown");
    THREADSAFE_CONNECT_UI_SIGNAL_TO_CORE_SIGNAL0(AppLifecycleManager::gApp->signalAppShutdownAlertUser, this, "shutdown_InitialUINotification");
    THREADSAFE_CONNECT_UI_SIGNAL_TO_CORE_SIGNAL0(AppLifecycleManager::gApp->signalAppShutdownCoreInterrupted, this, "shutdown_CloseModels");
    THREADSAFE_CONNECT_UI_SIGNAL_TO_CORE_SIGNAL0(AppLifecycleManager::gApp->signalAppShutdownFinished, this, "shutdown_TerminateApp");
}

GuldenApplication::~GuldenApplication()
{
    window = nullptr;
    platformStyle = nullptr;
    if (networkStyle)
    {
        delete networkStyle;
        networkStyle =nullptr;
    }
    shutdownWindow = nullptr;

    #ifdef ENABLE_WALLET
    delete paymentServer;
    paymentServer = nullptr;
    #endif
    delete optionsModel;
    optionsModel = nullptr;
}

#ifdef ENABLE_WALLET
void GuldenApplication::createPaymentServer()
{
    paymentServer = new PaymentServer(this);
}
#endif

void GuldenApplication::createOptionsModel(bool resetSettings)
{
    optionsModel = new OptionsModel(NULL, resetSettings);
}

void GuldenApplication::createWindow()
{
    window = new GUI(platformStyle, networkStyle, 0);
    connect( (QObject*)window->walletFrame, SIGNAL( loadWallet() ), this, SLOT( requestInitialize() ) );
    connect( window, SIGNAL( destroyed() ), this, SLOT( shutdown_MainWindowDestroyed() ) );

    // Handle underline hints for button shorcuts when alt key is down.
    GuldenEventFilter* guldenEventFilter = new GuldenEventFilter(this, platformStyle, window);
    guldenEventFilter->setObjectName("gui_event_filter");
    installEventFilter(guldenEventFilter);

    window->show();
}

void GuldenApplication::createNetworkStyle(QString networkIDString)
{
    networkStyle = const_cast<NetworkStyle*>(NetworkStyle::instantiate(networkIDString));
    assert(networkStyle);

    // Allow for separate UI settings for testnets
    setApplicationName(IsArgSet("-windowtitle") ? QString::fromStdString(GetArg("-windowtitle", "")) : networkStyle->getAppName());
}

void GuldenApplication::parameterSetup()
{
    InitLogging();
    InitParameterInteraction();
}

void GuldenApplication::requestInitialize()
{
    #ifdef LOG_ALL_QT_EVENTS
    logEvents = true;
    #endif
    qDebug() << __func__ << ": Requesting initialize";
    AppLifecycleManager::gApp->initialize();
}

static int returnValue = EXIT_FAILURE;
void GuldenApplication::initializeResult(bool success)
{
    qDebug() << __func__ << ": Initialization result: " << success;
    // Set exit result.
    returnValue = success ? EXIT_SUCCESS : EXIT_FAILURE;
    if(success && shutDownRequested==false)
    {
#ifdef ENABLE_WALLET
        PaymentServer::LoadRootCAs();
        paymentServer->setOptionsModel(optionsModel);
#endif

        clientModel = new ClientModel(optionsModel);
        window->setClientModel(clientModel);

#ifdef ENABLE_WALLET
        // TODO: Expose secondary wallets
        if (!vpwallets.empty())
        {
            walletModel = new WalletModel(platformStyle, vpwallets[0], optionsModel);

            window->addWallet(GUI::DEFAULT_WALLET, walletModel);
            window->setCurrentWallet(GUI::DEFAULT_WALLET);

            connect(walletModel, SIGNAL(coinsSent(CWallet*,SendCoinsRecipient,QByteArray)), paymentServer, SLOT(fetchPaymentACK(CWallet*,const SendCoinsRecipient&,QByteArray)));
        }
#endif

        // If -min option passed, start window minimized.
        if(GetBoolArg("-min", false))
        {
            window->showMinimized();
        }
        else
        {
            window->show();
        }

        window->autoUpdateCheck();

#ifdef ENABLE_WALLET
        // Now that initialization/startup is done, process any command-line
        // Gulden: URIs or payment requests:
        connect(paymentServer, SIGNAL(receivedPaymentRequest(SendCoinsRecipient)), window, SLOT(handlePaymentRequest(SendCoinsRecipient)));
        connect(window, SIGNAL(receivedURI(QString)), paymentServer, SLOT(handleURIOrFile(QString)));
        connect(paymentServer, SIGNAL(message(QString,QString,unsigned int)), window, SLOT(message(QString,QString,unsigned int)));
        QTimer::singleShot(100, paymentServer, SLOT(uiReady()));
#endif
    } else {
        quit(); // Exit main loop
    }
}

void GuldenApplication::shutdown_hideUIForShutdown()
{
    LogPrintf("shutdown UI: hide window for clean shutdown\n");

    window->hideForClose();
    shutDownRequested = true;
}

void GuldenApplication::shutdown_InitialUINotification()
{
    LogPrintf("shutdown UI: notify user of shutdown\n");
    shutdownWindow = ShutdownWindow::showShutdownWindow(window);

    LogPrintf("shutdown UI: disconnect wallet model from core signals\n");
    if (walletModel)
        walletModel->unsubscribeFromCoreSignals();

    LogPrintf("shutdown UI: disconnect client model from core signals\n");
    if (clientModel)
        clientModel->unsubscribeFromCoreSignals();

    translationInterface.Translate.disconnect(Translate);

    //fixme: (FUT) - disconnect transaction table model here as well?
}

void GuldenApplication::shutdown_CloseModels()
{
    LogPrintf("shutdown UI: disconnect client model\n");
    window->setClientModel(nullptr);

    //NB! We must delete the wallet model now already as otherwise core shutdown can delete the wallet while wallet model is still open.
    #ifdef ENABLE_WALLET
    LogPrintf("shutdown UI: remove all wallet models\n");
    window->removeAllWallets();
    LogPrintf("shutdown UI: delete all wallet models\n");
    if (walletModel)
    {
        walletModel->deleteLater();
        walletModel = nullptr;
    }
    #endif

    // Remove all timer slots now already so that they aren't a problem when we are shutting down.
    LogPrintf("shutdown UI: disconnect all timers and non-essential signals\n");
    window->disconnectNonEssentialSignals();

    LogPrintf("shutdown UI: delete client model\n");
    clientModel->deleteLater();
    clientModel = nullptr;
}

void GuldenApplication::shutdown_TerminateApp()
{
    // Finally exit main loop after shutdown finished
    //Signal the UI to shut itself down.
    LogPrintf("shutdown UI: all cleanup done, closing UI\n");
    window->coreAppIsReadyForUIToQuit=true;
    window->close();
    window = 0;
}

void GuldenApplication::shutdown_MainWindowDestroyed()
{
    // Close the final window (shutdown indicator), which will NOT automatically exit the Qt application
    LogPrintf("shutdown UI: GUI exited cleanly, closing shutdown indicator\n");
    ShutdownWindow::destroyInstance();

    // And finally explicitly exit Qt application
    quit();
}

void GuldenApplication::handleRunawayException(const QString &message)
{
    QMessageBox::critical(0, "Runaway exception", GUI::tr("A fatal error occurred. Gulden can no longer continue safely and will quit.") + QString("\n\n") + message);
    ::exit(EXIT_FAILURE);
}

WId GuldenApplication::getMainWinId() const
{
    if (!window)
        return 0;

    return window->winId();
}

#ifndef GULDEN_QT_TEST
int main(int argc, char *argv[])
{
    SetupEnvironment();

    /// 1. Parse command-line options. These take precedence over anything else.
    // Command-line options take precedence:
    ParseParameters(argc, argv);

    // Do not refer to data directory yet, this can be overridden by Intro::pickDataDirectory

    /// 2. Basic Qt initialization (not dependent on parameters or configuration)
#if QT_VERSION < 0x050000
    // Internal string conversion is all UTF-8
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForTr());
#endif

    Q_INIT_RESOURCE(gulden);
    Q_INIT_RESOURCE(gulden_locale);

    //NB!!! This must be called -before- the application is created, otherwise it has -no- effect.
#if QT_VERSION > 0x050100
    // Generate high-dpi pixmaps
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= 0x050600
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#ifdef Q_OS_MAC
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    AppLifecycleManager appManager;
    GuldenApplication* app = new GuldenApplication(argc, argv);
#if QT_VERSION >= 0x050500
    // Because of the POODLE attack it is recommended to disable SSLv3 (https://disablessl3.com/),
    // so set SSL protocols to TLS1.0+.
    QSslConfiguration sslconf = QSslConfiguration::defaultConfiguration();
    sslconf.setProtocol(QSsl::TlsV1_0OrLater);
    QSslConfiguration::setDefaultConfiguration(sslconf);
#endif

    // Register meta types used for QMetaObject::invokeMethod
    qRegisterMetaType<bool*>();
    qRegisterMetaType<CAccount*>();
    //   Need to pass name here as CAmount is a typedef (see http://qt-project.org/doc/qt-5/qmetatype.html#qRegisterMetaType)
    //   IMPORTANT if it is no longer a typedef use the normal variant above
    qRegisterMetaType<CAmount>("CAmount");
    qRegisterMetaType<WalletBalances>();
    qRegisterMetaType<std::function<void (void)>>();
    //Used by QVariant in table models.
    qRegisterMetaType<boost::uuids::uuid>();

    /// 3. Application identification
    // must be set before OptionsModel is initialized or translations are loaded,
    // as it is used to locate QSettings
    QApplication::setOrganizationName(QAPP_ORG_NAME);
    QApplication::setOrganizationDomain(QAPP_ORG_DOMAIN);
    QApplication::setApplicationName(QAPP_APP_NAME_DEFAULT);
    GUIUtil::SubstituteFonts(GetLangTerritory());

    /// 4. Initialization of translations, so that intro dialog is in user's language
    // Now that QSettings are accessible, initialize translations
    QTranslator qtTranslatorBase, qtTranslator, translatorBase, translator;

    initTranslations(qtTranslatorBase, qtTranslator, translatorBase, translator);
    translationInterface.Translate.connect(Translate);

    // Show help message immediately after parsing command-line options (for "-lang") and setting locale,
    if (IsArgSet("-?") || IsArgSet("-h") || IsArgSet("-help") || IsArgSet("-version"))
    {
        HelpMessageDialog help(NULL, IsArgSet("-version"));
        help.showOrPrint();
        return EXIT_SUCCESS;
    }

    /// 5. Now that settings and translations are available, ask user for data directory
    // User language is set up: pick a data directory
    if (!Intro::pickDataDirectory())
        return EXIT_SUCCESS;

    /// 6. Determine availability of data directory and parse Gulden.conf
    /// - Do not call GetDataDir(true) before this step finishes
    if (!fs::is_directory(GetDataDir(false)))
    {
        QMessageBox::critical(0, QObject::tr(PACKAGE_NAME),
                              QObject::tr("Error: Specified data directory \"%1\" does not exist.").arg(QString::fromStdString(GetArg("-datadir", ""))));
        return EXIT_FAILURE;
    }
    try {
        ReadConfigFile(GetArg("-conf", DEFAULT_CONF_FILENAME));
    } catch (const std::exception& e) {
        QMessageBox::critical(0, QObject::tr(PACKAGE_NAME),
                              QObject::tr("Error: Cannot parse configuration file: %1. Only use key=value syntax.").arg(e.what()));
        return EXIT_FAILURE;
    }

    /// 7. Determine network (and switch to network specific options)
    // - Do not call Params() before this step
    // - Do this after parsing the configuration file, as the network can be switched there
    // - QSettings() will use the new application name after this, resulting in network-specific settings
    // - Needs to be done before createOptionsModel

    // Check for -testnet or -regtest parameter (Params() calls are only valid after this clause)
    try {
        SelectParams(ChainNameFromCommandLine());
    } catch(std::exception &e) {
        QMessageBox::critical(0, QObject::tr(PACKAGE_NAME), QObject::tr("Error: %1").arg(e.what()));
        return EXIT_FAILURE;
    }
#ifdef ENABLE_WALLET
    // Parse URIs on command line -- this can affect Params()
    PaymentServer::ipcParseCommandLine(argc, argv);
#endif

    app->createNetworkStyle(QString::fromStdString(Params().NetworkIDString()));

    // Re-initialize translations after changing application name (language in network-specific settings can be different)
    initTranslations(qtTranslatorBase, qtTranslator, translatorBase, translator);

#ifdef ENABLE_WALLET
    /// 8. URI IPC sending
    // - Do this early as we don't want to bother initializing if we are just calling IPC
    // - Do this *after* setting up the data directory, as the data directory hash is used in the name
    // of the server.
    // - Do this after creating app and setting up translations, so errors are
    // translated properly.
    if (PaymentServer::ipcSendCommandLine())
        exit(EXIT_SUCCESS);

    // Start up the payment server early, too, so impatient users that click on
    // Gulden: links repeatedly have their payment requests routed to this process:
    app->createPaymentServer();
#endif

    /// 9. Main GUI initialization
    // Install global event filter that makes sure that long tooltips can be word-wrapped
    app->installEventFilter(new GUIUtil::ToolTipToRichTextFilter(TOOLTIP_WRAP_THRESHOLD, app));
#if QT_VERSION < 0x050000
    // Install qDebug() message handler to route to debug.log
    qInstallMsgHandler(DebugMessageHandler);
#else
#if defined(Q_OS_WIN)
    // Install global event filter for processing Windows session related Windows messages (WM_QUERYENDSESSION and WM_ENDSESSION)
    qApp->installNativeEventFilter(new WinShutdownMonitor());
#endif
    // Install qDebug() message handler to route to debug.log
    qInstallMessageHandler(DebugMessageHandler);
#endif
    // Allow parameter interaction before we create the options model
    app->parameterSetup();
    // Load GUI settings from QSettings
    app->createOptionsModel(IsArgSet("-resetguisettings"));

    // Subscribe to global signals from core
    uiInterface.InitMessage.connect(InitMessage);

    //fixme: (FUT) - This is now duplicated, factor this out into a common helper.
    // Make sure only a single Gulden process is using the data directory.
    fs::path pathLockFile = GetDataDir() / ".lock";
    FILE* file = fopen(pathLockFile.string().c_str(), "a"); // empty lock file; created if it doesn't exist.
    if (file) fclose(file);

    try
    {
        static boost::interprocess::file_lock lock(pathLockFile.string().c_str());
        if (!lock.try_lock())
        {
            QMessageBox::critical(0, QObject::tr(PACKAGE_NAME), QString::fromStdString(strprintf(_("ERROR: Cannot obtain a lock on data directory %s. %s is probably already running."), GetDataDir().string(), _(PACKAGE_NAME))));
            return 1;
        }
    }
    catch(const boost::interprocess::interprocess_exception& e)
    {
        QMessageBox::critical(0, QObject::tr(PACKAGE_NAME), QString::fromStdString(strprintf(_("ERROR: Cannot obtain a lock on data directory %s. %s is probably already running."), GetDataDir().string(), _(PACKAGE_NAME))));
        return 1;
    }

    try
    {
        app->createWindow();
        #if defined(Q_OS_WIN) && QT_VERSION >= 0x050000
        WinShutdownMonitor::registerShutdownBlockReason(QObject::tr("%1 didn't yet exit safely...").arg(QObject::tr(PACKAGE_NAME)), (HWND)app->getMainWinId());
        #endif
        app->exec();
        //NB! App is self-deleting so invalid past this point.
        app = nullptr;
        LogPrintf("%s: UI shutdown done, terminating.\n", __func__);
    }
    catch (const std::exception& e)
    {
        PrintExceptionContinue(&e, "Runaway exception");
        app->handleRunawayException(QString::fromStdString(GetWarnings("gui")));
    }
    catch (...)
    {
        PrintExceptionContinue(NULL, "Runaway exception");
        app->handleRunawayException(QString::fromStdString(GetWarnings("gui")));
    }
    return returnValue;
}
#endif // GULDEN_QT_TEST

//fixme: (HIGH)
//Super gross workaround - for some reason our macos build keeps failing to provide '___cpu_model' symbol, so we just define it ourselves as a workaround until we can fix the issue.
#ifdef MAC_OSX
#include "llvm-cpumodel-hack.cpp"
#endif
