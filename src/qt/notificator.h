// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GULDEN_QT_NOTIFICATOR_H
#define GULDEN_QT_NOTIFICATOR_H

#if defined(HAVE_CONFIG_H)
#include "config/gulden-config.h"
#endif

#include <QIcon>
#include <QObject>

QT_BEGIN_NAMESPACE
class QSystemTrayIcon;

#ifdef USE_DBUS
class QDBusInterface;
#endif
QT_END_NAMESPACE

/** Cross-platform desktop notification client. */
class Notificator: public QObject
{
    Q_OBJECT

public:
    /** Create a new notificator.
       @note Ownership of trayIcon is not transferred to this object.
    */
    Notificator(const QString &programName, QSystemTrayIcon *trayIcon, QWidget *parent);
    ~Notificator();

    // Message class
    enum Class
    {
        Information,    /**< Informational message */
        Warning,        /**< Notify user of potential problem */
        Critical        /**< An error occurred */
    };

public Q_SLOTS:
    /** Show notification message.
       @param[in] cls    general message class
       @param[in] title  title shown with message
       @param[in] text   message content
       @param[in] icon   optional icon to show with message
       @param[in] millisTimeout notification timeout in milliseconds (defaults to 10 seconds)
       @note Platform implementations are free to ignore any of the provided fields except for \a text.
     */
    void notify(Class cls, const QString &title, const QString &text,
                const QIcon &icon = QIcon(), int millisTimeout = 10000);

private:
    QWidget *parent;
    enum Mode {
        None                       /**< Ignore informational notifications, and show a modal pop-up dialog for Critical notifications. */
        ,Freedesktop                /**< Use DBus org.freedesktop.Notifications */
        ,QSystemTray                /**< Use QSystemTray::showMessage */
        #ifdef Q_OS_MAC
        ,Growl12                    /**< Use the Growl 1.2 notification system (Mac only) */
        ,Growl13                    /**< Use the Growl 1.3 notification system (Mac only) */
        ,UserNotificationCenter      /**< Use the 10.8+ User Notification Center (Mac only) */
        #endif
    };
    QString programName;
    Mode mode;
    QSystemTrayIcon *trayIcon;
#ifdef USE_DBUS
    QDBusInterface *interface;

    void notifyDBus(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout);
#endif
    void notifySystray(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout);
#ifdef Q_OS_MAC
    void notifyGrowl(Class cls, const QString &title, const QString &text, const QIcon &icon);
    void notifyMacUserNotificationCenter(Class cls, const QString &title, const QString &text, const QIcon &icon);
#endif
};

#endif // GULDEN_QT_NOTIFICATOR_H
