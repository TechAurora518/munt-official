/*
 * W.J. van der Laan 2011-2012
 */
#include "bitcoingui.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include "init.h"
#include "ui_interface.h"
#include "qtipcserver.h"

#include <QApplication>
#include <QMessageBox>
#include <QTextCodec>
#include <QLocale>
#include <QTranslator>
#include <QSplashScreen>
#include <QLibraryInfo>

#include <boost/interprocess/ipc/message_queue.hpp>

#if defined(BITCOIN_NEED_QT_PLUGINS) && !defined(_BITCOIN_QT_PLUGINS_INCLUDED)
#define _BITCOIN_QT_PLUGINS_INCLUDED
#define __INSURE__
#include <QtPlugin>
Q_IMPORT_PLUGIN(qcncodecs)
Q_IMPORT_PLUGIN(qjpcodecs)
Q_IMPORT_PLUGIN(qtwcodecs)
Q_IMPORT_PLUGIN(qkrcodecs)
Q_IMPORT_PLUGIN(qtaccessiblewidgets)
#endif

// Need a global reference for the notifications to find the GUI
static BitcoinGUI *guiref;
static QSplashScreen *splashref;
static WalletModel *walletmodel;
static ClientModel *clientmodel;

int ThreadSafeMessageBox(const std::string& message, const std::string& caption, int style)
{
    // Message from network thread
    if(guiref)
    {
        bool modal = (style & wxMODAL);
        // in case of modal message, use blocking connection to wait for user to click OK
        QMetaObject::invokeMethod(guiref, "error",
                                   modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
                                   Q_ARG(QString, QString::fromStdString(caption)),
                                   Q_ARG(QString, QString::fromStdString(message)),
                                   Q_ARG(bool, modal));
    }
    else
    {
        printf("%s: %s\n", caption.c_str(), message.c_str());
        fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
    }
    return 4;
}

bool ThreadSafeAskFee(int64 nFeeRequired, const std::string& strCaption)
{
    if(!guiref)
        return false;
    if(nFeeRequired < MIN_TX_FEE || nFeeRequired <= nTransactionFee || fDaemon)
        return true;
    bool payFee = false;

    QMetaObject::invokeMethod(guiref, "askFee", GUIUtil::blockingGUIThreadConnection(),
                               Q_ARG(qint64, nFeeRequired),
                               Q_ARG(bool*, &payFee));

    return payFee;
}

void ThreadSafeHandleURI(const std::string& strURI)
{
    if(!guiref)
        return;

    QMetaObject::invokeMethod(guiref, "handleURI", GUIUtil::blockingGUIThreadConnection(),
                               Q_ARG(QString, QString::fromStdString(strURI)));
}

void InitMessage(const std::string &message)
{
    if(splashref)
    {
        splashref->showMessage(QString::fromStdString(message), Qt::AlignBottom|Qt::AlignHCenter, QColor(255,255,200));
        QApplication::instance()->processEvents();
    }
}

void QueueShutdown()
{
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
}

/*
   Translate string to current locale using Qt.
 */
std::string _(const char* psz)
{
    return QCoreApplication::translate("bitcoin-core", psz).toStdString();
}

void NotifyBlocksChanged()
{
    // This notification is too frequent. Don't trigger a signal.
    // Don't remove it, though, as it might be useful later.
}

void NotifyKeyStoreStatusChanged(CBasicKeyStore *wallet)
{
    // This currently ignores the wallet argument. When multiple wallet support is implemented, this
    // parameter should be mapped to a specific WalletModel for that wallet.
    OutputDebugStringF("NotifyKeyStoreStatusChanged\n");
    if(walletmodel)
        QMetaObject::invokeMethod(walletmodel, "updateStatus", Qt::QueuedConnection);
}

void NotifyAddressBookChanged(CWallet *wallet, const std::string &address, const std::string &label, ChangeType status)
{
    // This currently ignores the wallet argument. When multiple wallet support is implemented, this
    // parameter should be mapped to a specific WalletModel for that wallet.
    OutputDebugStringF("NotifyAddressBookChanged %s %s status=%i\n", address.c_str(), label.c_str(), status);
    if(walletmodel)
        QMetaObject::invokeMethod(walletmodel, "updateAddressBook", Qt::QueuedConnection,
                                  Q_ARG(QString, QString::fromStdString(address)),
                                  Q_ARG(QString, QString::fromStdString(label)),
                                  Q_ARG(int, status));
}

void NotifyTransactionChanged(CWallet *wallet, const uint256 &hash, ChangeType status)
{
    // This currently ignores the wallet argument. When multiple wallet support is implemented, this
    // parameter should be mapped to a specific WalletModel for that wallet.
    OutputDebugStringF("NotifyTransactionChanged %s status=%i\n", hash.GetHex().c_str(), status);
    if(walletmodel)
        QMetaObject::invokeMethod(walletmodel, "updateTransaction", Qt::QueuedConnection,
                                  Q_ARG(QString, QString::fromStdString(hash.GetHex())),
                                  Q_ARG(int, status));
}

void NotifyNumConnectionsChanged(int newNumConnections)
{
    // Too noisy: OutputDebugStringF("NotifyNumConnectionsChanged %i\n", newNumConnections);
    if(clientmodel)
        QMetaObject::invokeMethod(clientmodel, "updateNumConnections", Qt::QueuedConnection,
                                  Q_ARG(int, newNumConnections));
}

void NotifyAlertChanged(const uint256 &hash, ChangeType status)
{
    OutputDebugStringF("NotifyAlertChanged %s status=%i\n", hash.GetHex().c_str(), status);
    if(clientmodel)
        QMetaObject::invokeMethod(clientmodel, "updateAlert", Qt::QueuedConnection,
                                  Q_ARG(QString, QString::fromStdString(hash.GetHex())),
                                  Q_ARG(int, status));
}

/* Handle runaway exceptions. Shows a message box with the problem and quits the program.
 */
static void handleRunawayException(std::exception *e)
{
    PrintExceptionContinue(e, "Runaway exception");
    QMessageBox::critical(0, "Runaway exception", BitcoinGUI::tr("A fatal error occured. Bitcoin can no longer continue safely and will quit.") + QString("\n\n") + QString::fromStdString(strMiscWarning));
    exit(1);
}

/** Help message for Bitcoin-Qt, shown with --help. */
class HelpMessageBox: public QMessageBox
{
    Q_OBJECT
public:
    HelpMessageBox(QWidget *parent = 0);

    void exec();
private:
    QString header;
    QString coreOptions;
    QString uiOptions;
};

HelpMessageBox::HelpMessageBox(QWidget *parent):
    QMessageBox(parent)
{
    header = tr("Bitcoin-Qt") + " " + tr("version") + " " +
        QString::fromStdString(FormatFullVersion()) + "\n\n" +
        tr("Usage:") + "\n" +
        "  bitcoin-qt [" + tr("options") + "]                     " + "\n";
    coreOptions = QString::fromStdString(HelpMessage());
    uiOptions = tr("UI options") + ":\n" +
        "  -lang=<lang>           " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        "  -min                   " + tr("Start minimized") + "\n" +
        "  -splash                " + tr("Show splash screen on startup (default: 1)") + "\n";

    setWindowTitle(tr("Bitcoin-Qt"));
    setTextFormat(Qt::PlainText);
    // setMinimumWidth is ignored for QMessageBox so put in nonbreaking spaces to make it wider.
    QChar em_space(0x2003);
    setText(header + QString(em_space).repeated(40));
    setDetailedText(coreOptions + "\n" + uiOptions);
}
#include "bitcoin.moc"

void HelpMessageBox::exec()
{
#if defined(WIN32)
    // On windows, show a message box, as there is no stderr in windowed applications
    QMessageBox::exec();
#else
    // On other operating systems, the expected action is to print the message to the console.
    QString strUsage = header + "\n" + coreOptions + "\n" + uiOptions;
    fprintf(stderr, "%s", strUsage.toStdString().c_str());
#endif
}

#ifdef WIN32
#define strncasecmp strnicmp
#endif
#ifndef BITCOIN_QT_TEST
int main(int argc, char *argv[])
{
#if !defined(MAC_OSX) && !defined(WIN32)
// TODO: implement qtipcserver.cpp for Mac and Windows

    // Do this early as we don't want to bother initializing if we are just calling IPC
    for (int i = 1; i < argc; i++)
    {
        if (strlen(argv[i]) > 7 && strncasecmp(argv[i], "bitcoin:", 8) == 0)
        {
            const char *strURI = argv[i];
            try {
                boost::interprocess::message_queue mq(boost::interprocess::open_only, BITCOINURI_QUEUE_NAME);
                if(mq.try_send(strURI, strlen(strURI), 0))
                    exit(0);
                else
                    break;
            }
            catch (boost::interprocess::interprocess_exception &ex) {
                break;
            }
        }
    }
#endif

    // Internal string conversion is all UTF-8
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForTr());

    Q_INIT_RESOURCE(bitcoin);
    QApplication app(argc, argv);

    // Install global event filter that makes sure that long tooltips can be word-wrapped
    app.installEventFilter(new GUIUtil::ToolTipToRichTextFilter(TOOLTIP_WRAP_THRESHOLD, &app));

    // Command-line options take precedence:
    ParseParameters(argc, argv);

    // ... then bitcoin.conf:
    if (!boost::filesystem::is_directory(GetDataDir(false)))
    {
        fprintf(stderr, "Error: Specified directory does not exist\n");
        return 1;
    }
    ReadConfigFile(mapArgs, mapMultiArgs);

    // Application identification (must be set before OptionsModel is initialized,
    // as it is used to locate QSettings)
    app.setOrganizationName("Bitcoin");
    app.setOrganizationDomain("bitcoin.org");
    if(GetBoolArg("-testnet")) // Separate UI settings for testnet
        app.setApplicationName("Bitcoin-Qt-testnet");
    else
        app.setApplicationName("Bitcoin-Qt");

    // ... then GUI settings:
    OptionsModel optionsModel;

    // Get desired locale (e.g. "de_DE") from command line or use system locale
    QString lang_territory = QString::fromStdString(GetArg("-lang", QLocale::system().name().toStdString()));
    QString lang = lang_territory;
    // Convert to "de" only by truncating "_DE"
    lang.truncate(lang_territory.lastIndexOf('_'));

    QTranslator qtTranslatorBase, qtTranslator, translatorBase, translator;
    // Load language files for configured locale:
    // - First load the translator for the base language, without territory
    // - Then load the more specific locale translator

    // Load e.g. qt_de.qm
    if (qtTranslatorBase.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslatorBase);

    // Load e.g. qt_de_DE.qm
    if (qtTranslator.load("qt_" + lang_territory, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslator);

    // Load e.g. bitcoin_de.qm (shortcut "de" needs to be defined in bitcoin.qrc)
    if (translatorBase.load(lang, ":/translations/"))
        app.installTranslator(&translatorBase);

    // Load e.g. bitcoin_de_DE.qm (shortcut "de_DE" needs to be defined in bitcoin.qrc)
    if (translator.load(lang_territory, ":/translations/"))
        app.installTranslator(&translator);

    // Show help message immediately after parsing command-line options (for "-lang") and setting locale,
    // but before showing splash screen.
    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
        HelpMessageBox help;
        help.exec();
        return 1;
    }

    QSplashScreen splash(QPixmap(":/images/splash"), 0);
    if (GetBoolArg("-splash", true) && !GetBoolArg("-min"))
    {
        splash.show();
        splash.setAutoFillBackground(true);
        splashref = &splash;
    }

    app.processEvents();

    app.setQuitOnLastWindowClosed(false);

    try
    {
        // Regenerate startup link, to fix links to old versions
        if (GUIUtil::GetStartOnSystemStartup())
            GUIUtil::SetStartOnSystemStartup(true);

        BitcoinGUI window;
        guiref = &window;
        if(AppInit2())
        {
            {
                // Put this in a block, so that the Model objects are cleaned up before
                // calling Shutdown().

                optionsModel.Upgrade(); // Must be done after AppInit2

                if (splashref)
                    splash.finish(&window);

                ClientModel clientModel(&optionsModel);
                clientmodel = &clientModel;
                WalletModel walletModel(pwalletMain, &optionsModel);
                walletmodel = &walletModel;

                window.setClientModel(&clientModel);
                window.setWalletModel(&walletModel);

                // If -min option passed, start window minimized.
                if(GetBoolArg("-min"))
                {
                    window.showMinimized();
                }
                else
                {
                    window.show();
                }

                // Place this here as guiref has to be defined if we dont want to lose URIs
                ipcInit();

#if !defined(MAC_OSX) && !defined(WIN32)
// TODO: implement qtipcserver.cpp for Mac and Windows

                // Check for URI in argv
                for (int i = 1; i < argc; i++)
                {
                    if (strlen(argv[i]) > 7 && strncasecmp(argv[i], "bitcoin:", 8) == 0)
                    {
                        const char *strURI = argv[i];
                        try {
                            boost::interprocess::message_queue mq(boost::interprocess::open_only, BITCOINURI_QUEUE_NAME);
                            mq.try_send(strURI, strlen(strURI), 0);
                        }
                        catch (boost::interprocess::interprocess_exception &ex) {
                        }
                    }
                }
#endif
                app.exec();

                window.hide();
                window.setClientModel(0);
                window.setWalletModel(0);
                guiref = 0;
                clientmodel = 0;
                walletmodel = 0;
            }
            Shutdown(NULL);
        }
        else
        {
            return 1;
        }
    } catch (std::exception& e) {
        handleRunawayException(&e);
    } catch (...) {
        handleRunawayException(NULL);
    }
    return 0;
}
#endif // BITCOIN_QT_TEST
