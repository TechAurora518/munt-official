// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2012-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_UI_INTERFACE_H
#define GULDEN_UI_INTERFACE_H


#include <stdint.h>
#include <string>

#include <boost/signals2/last_value.hpp>
#include <boost/signals2/signal.hpp>

class CBasicKeyStore;
class CWallet;
class uint256;
class CBlockIndex;

/** General change type (added, updated, removed). */
enum ChangeType
{
    CT_NEW,
    CT_UPDATED,
    CT_DELETED
};

/** Signals for UI communication. */
class CClientUIInterface
{
public:
    /** Flags for CClientUIInterface::ThreadSafeMessageBox */
    enum MessageBoxFlags
    {
        ICON_INFORMATION    = 0,
        ICON_WARNING        = (1U << 0),
        ICON_ERROR          = (1U << 1),
        /**
         * Mask of all available icons in CClientUIInterface::MessageBoxFlags
         * This needs to be updated, when icons are changed there!
         */
        ICON_MASK = (ICON_INFORMATION | ICON_WARNING | ICON_ERROR),

        /** These values are taken from qmessagebox.h "enum StandardButton" to be directly usable */
        BTN_OK      = 0x00000400U, // QMessageBox::Ok
        BTN_YES     = 0x00004000U, // QMessageBox::Yes
        BTN_NO      = 0x00010000U, // QMessageBox::No
        BTN_ABORT   = 0x00040000U, // QMessageBox::Abort
        BTN_RETRY   = 0x00080000U, // QMessageBox::Retry
        BTN_IGNORE  = 0x00100000U, // QMessageBox::Ignore
        BTN_CLOSE   = 0x00200000U, // QMessageBox::Close
        BTN_CANCEL  = 0x00400000U, // QMessageBox::Cancel
        BTN_DISCARD = 0x00800000U, // QMessageBox::Discard
        BTN_HELP    = 0x01000000U, // QMessageBox::Help
        BTN_APPLY   = 0x02000000U, // QMessageBox::Apply
        BTN_RESET   = 0x04000000U, // QMessageBox::Reset
        /**
         * Mask of all available buttons in CClientUIInterface::MessageBoxFlags
         * This needs to be updated, when buttons are changed there!
         */
        BTN_MASK = (BTN_OK | BTN_YES | BTN_NO | BTN_ABORT | BTN_RETRY | BTN_IGNORE |
                    BTN_CLOSE | BTN_CANCEL | BTN_DISCARD | BTN_HELP | BTN_APPLY | BTN_RESET),

        /** Force blocking, modal message box dialog (not just OS notification) */
        MODAL               = 0x10000000U,

        /** Do not print contents of message to debug log */
        SECURE              = 0x40000000U,

        /** Predefined combinations for certain default usage cases */
        MSG_INFORMATION = ICON_INFORMATION,
        MSG_WARNING = (ICON_WARNING | BTN_OK | MODAL),
        MSG_ERROR = (ICON_ERROR | BTN_OK | MODAL)
    };

    /** Show message box. */
    boost::signals2::signal<bool (const std::string& message, const std::string& caption, unsigned int style) > ThreadSafeMessageBox;

    /** If possible, ask the user a question. If not, falls back to ThreadSafeMessageBox(noninteractive_message, caption, style) and returns false. */
    boost::signals2::signal<bool (const std::string& message, const std::string& noninteractive_message, const std::string& caption, unsigned int style), boost::signals2::optional_last_value<bool> > ThreadSafeQuestion;

    /** Progress message during initialization. */
    boost::signals2::signal<void (const std::string &message)> InitMessage;

    /** Number of network connections changed. */
    boost::signals2::signal<void (int newNumConnections)> NotifyNumConnectionsChanged;

    /** Network activity state changed. */
    boost::signals2::signal<void (bool networkActive)> NotifyNetworkActiveChanged;

    /**
     * Status bar alerts changed.
     */
    boost::signals2::signal<void (const uint256 &hash, ChangeType status)> NotifyAlertChanged;

    //! Alert for user (top of UI) changed.
    boost::signals2::signal<void (const std::string& alertMessage)> NotifyUIAlertChanged;

    /** A wallet has been loaded. */
    boost::signals2::signal<void (CWallet* wallet)> LoadWallet;

    /** The core requires a wallet unlock. */
    boost::signals2::signal<void (CWallet* wallet, std::string reason)> RequestUnlock;
    boost::signals2::signal<void (CWallet* wallet, std::string reason, std::function<void (void)> successCallback)> RequestUnlockWithCallback;

    /** Show progress e.g. for verifychain */
    boost::signals2::signal<void (const std::string &title, int nProgress)> ShowProgress;

    /** New block has been accepted */
    boost::signals2::signal<void (bool, const CBlockIndex *)> NotifyBlockTip;

    /** Header progress has changed */
    boost::signals2::signal<void (int, int, int, int64_t)> NotifyHeaderProgress;

    /** Banlist did change. */
    boost::signals2::signal<void (void)> BannedListChanged;

    /** SPV progress */
    boost::signals2::signal<void (int start_height, int processed_height, int probable_height)> NotifySPVProgress;

    /** unified progress */
    boost::signals2::signal<void (float progress)> NotifyUnifiedProgress;

    /** SPV pruning */
    boost::signals2::signal<void (int height)> NotifySPVPrune;
};

/** Show warning message **/
void InitWarning(const std::string& str);

/** Show error message **/
bool InitError(const std::string& str);

std::string AmountHighWarn(const std::string& optname);

std::string AmountErrMsg(const char* const optname, const std::string& strValue);

extern CClientUIInterface uiInterface;

#endif // GULDEN_UI_INTERFACE_H
