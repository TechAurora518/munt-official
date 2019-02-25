// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

/** This interface will be implemented in C++ and can be called from any language. */
public abstract class GuldenUnifiedBackend {
    /** Interface constants */
    public static final int VERSION = 1;

    /** Start the library */
    public static int InitUnityLib(String dataDir, boolean testnet, GuldenUnifiedFrontend signals)
    {
        return CppProxy.InitUnityLib(dataDir,
                                     testnet,
                                     signals);
    }

    /** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
    public static boolean InitWalletFromRecoveryPhrase(String phrase)
    {
        return CppProxy.InitWalletFromRecoveryPhrase(phrase);
    }

    /** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
    public static boolean InitWalletLinkedFromURI(String linkedUri)
    {
        return CppProxy.InitWalletLinkedFromURI(linkedUri);
    }

    /** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
    public static boolean InitWalletFromAndroidLegacyProtoWallet(String walletFile, String password)
    {
        return CppProxy.InitWalletFromAndroidLegacyProtoWallet(walletFile,
                                                               password);
    }

    /** Check if a file is a valid legacy proto wallet */
    public static LegacyWalletResult isValidAndroidLegacyProtoWallet(String walletFile, String password)
    {
        return CppProxy.isValidAndroidLegacyProtoWallet(walletFile,
                                                        password);
    }

    /** Check link URI for validity */
    public static boolean IsValidLinkURI(String phrase)
    {
        return CppProxy.IsValidLinkURI(phrase);
    }

    /** Replace the existing wallet accounts with a new one from a linked URI - only after first emptying the wallet. */
    public static boolean ReplaceWalletLinkedFromURI(String linkedUri)
    {
        return CppProxy.ReplaceWalletLinkedFromURI(linkedUri);
    }

    /**
     * Erase the seeds and accounts of a wallet leaving an empty wallet (with things like the address book intact)
     * After calling this it will be necessary to create a new linked account or recovery phrase account again.
     * NB! This will empty a wallet regardless of whether it has funds in it or not and makes no provisions to check for this - it is the callers responsibility to ensure that erasing the wallet is safe to do in this regard.
     */
    public static boolean EraseWalletSeedsAndAccounts()
    {
        return CppProxy.EraseWalletSeedsAndAccounts();
    }

    /**
     * Check recovery phrase for (syntactic) validity
     * Considered valid if the contained mnemonic is valid and the birth-number is either absent or passes Base-10 checksum
     */
    public static boolean IsValidRecoveryPhrase(String phrase)
    {
        return CppProxy.IsValidRecoveryPhrase(phrase);
    }

    /** Generate a new recovery mnemonic */
    public static String GenerateRecoveryMnemonic()
    {
        return CppProxy.GenerateRecoveryMnemonic();
    }

    /** Compute recovery phrase with birth number */
    public static String ComposeRecoveryPhrase(String mnemonic, long birthTime)
    {
        return CppProxy.ComposeRecoveryPhrase(mnemonic,
                                              birthTime);
    }

    /** Stop the library */
    public static void TerminateUnityLib()
    {
        CppProxy.TerminateUnityLib();
    }

    /** Generate a QR code for a string, QR code will be as close to widthHint as possible when applying simple scaling. */
    public static QrCodeRecord QRImageFromString(String qrString, int widthHint)
    {
        return CppProxy.QRImageFromString(qrString,
                                          widthHint);
    }

    /** Get a receive address from the wallet */
    public static String GetReceiveAddress()
    {
        return CppProxy.GetReceiveAddress();
    }

    /** Get the recovery phrase for the wallet */
    public static String GetRecoveryPhrase()
    {
        return CppProxy.GetRecoveryPhrase();
    }

    /** Check if the wallet is using a mnemonic seed ie. recovery phrase (else it is a linked wallet) */
    public static boolean IsMnemonicWallet()
    {
        return CppProxy.IsMnemonicWallet();
    }

    /** Check if the phrase mnemonic is a correct one for the wallet (phrase can be with or without birth time) */
    public static boolean IsMnemonicCorrect(String phrase)
    {
        return CppProxy.IsMnemonicCorrect(phrase);
    }

    /** Check if the wallet has any transactions that are still pending confirmation, to be used to determine if e.g. it is safe to perform a link or whether we should wait. */
    public static boolean HaveUnconfirmedFunds()
    {
        return CppProxy.HaveUnconfirmedFunds();
    }

    /** Check current wallet balance (including unconfirmed funds) */
    public static long GetBalance()
    {
        return CppProxy.GetBalance();
    }

    /** Rescan blockchain for wallet transactions */
    public static void DoRescan()
    {
        CppProxy.DoRescan();
    }

    /** Check if text/address is something we are capable of sending money too */
    public static UriRecipient IsValidRecipient(UriRecord request)
    {
        return CppProxy.IsValidRecipient(request);
    }

    /** Attempt to pay a recipient, will throw on failure with description */
    public static void performPaymentToRecipient(UriRecipient request)
    {
        CppProxy.performPaymentToRecipient(request);
    }

    /** Get list of all transactions wallet has been involved in */
    public static ArrayList<TransactionRecord> getTransactionHistory()
    {
        return CppProxy.getTransactionHistory();
    }

    /**
     * Get the wallet transaction for the hash
     * Will throw if not found
     */
    public static TransactionRecord getTransaction(String txHash)
    {
        return CppProxy.getTransaction(txHash);
    }

    /** Get list of wallet mutations */
    public static ArrayList<MutationRecord> getMutationHistory()
    {
        return CppProxy.getMutationHistory();
    }

    /** Get list of all address book entries */
    public static ArrayList<AddressRecord> getAddressBookRecords()
    {
        return CppProxy.getAddressBookRecords();
    }

    /** Add a record to the address book */
    public static void addAddressBookRecord(AddressRecord address)
    {
        CppProxy.addAddressBookRecord(address);
    }

    /** Delete a record from the address book */
    public static void deleteAddressBookRecord(AddressRecord address)
    {
        CppProxy.deleteAddressBookRecord(address);
    }

    /** Interim persist and prune of state. Use at key moments like app backgrounding. */
    public static void PersistAndPruneForSPV()
    {
        CppProxy.PersistAndPruneForSPV();
    }

    /**
     * Reset progress notification. In cases where there has been no progress for a long time, but the process
     * is still running the progress can be reset and will represent work to be done from this reset onwards.
     * For example when the process is in the background on iOS for a long long time (but has not been terminated
     * by the OS) this might make more sense then to continue the progress from where it was a day or more ago.
     */
    public static void ResetUnifiedProgress()
    {
        CppProxy.ResetUnifiedProgress();
    }

    /** Get connected peer info */
    public static ArrayList<PeerRecord> getPeers()
    {
        return CppProxy.getPeers();
    }

    /** Get info of last blocks (at most 32) in SPV chain */
    public static ArrayList<BlockInfoRecord> getLastSPVBlockInfos()
    {
        return CppProxy.getLastSPVBlockInfos();
    }

    public static MonitorRecord getMonitoringStats()
    {
        return CppProxy.getMonitoringStats();
    }

    public static void RegisterMonitorListener(GuldenMonitorListener listener)
    {
        CppProxy.RegisterMonitorListener(listener);
    }

    public static void UnregisterMonitorListener(GuldenMonitorListener listener)
    {
        CppProxy.UnregisterMonitorListener(listener);
    }

    private static final class CppProxy extends GuldenUnifiedBackend
    {
        private final long nativeRef;
        private final AtomicBoolean destroyed = new AtomicBoolean(false);

        private CppProxy(long nativeRef)
        {
            if (nativeRef == 0) throw new RuntimeException("nativeRef is zero");
            this.nativeRef = nativeRef;
        }

        private native void nativeDestroy(long nativeRef);
        public void _djinni_private_destroy()
        {
            boolean destroyed = this.destroyed.getAndSet(true);
            if (!destroyed) nativeDestroy(this.nativeRef);
        }
        protected void finalize() throws java.lang.Throwable
        {
            _djinni_private_destroy();
            super.finalize();
        }

        public static native int InitUnityLib(String dataDir, boolean testnet, GuldenUnifiedFrontend signals);

        public static native boolean InitWalletFromRecoveryPhrase(String phrase);

        public static native boolean InitWalletLinkedFromURI(String linkedUri);

        public static native boolean InitWalletFromAndroidLegacyProtoWallet(String walletFile, String password);

        public static native LegacyWalletResult isValidAndroidLegacyProtoWallet(String walletFile, String password);

        public static native boolean IsValidLinkURI(String phrase);

        public static native boolean ReplaceWalletLinkedFromURI(String linkedUri);

        public static native boolean EraseWalletSeedsAndAccounts();

        public static native boolean IsValidRecoveryPhrase(String phrase);

        public static native String GenerateRecoveryMnemonic();

        public static native String ComposeRecoveryPhrase(String mnemonic, long birthTime);

        public static native void TerminateUnityLib();

        public static native QrCodeRecord QRImageFromString(String qrString, int widthHint);

        public static native String GetReceiveAddress();

        public static native String GetRecoveryPhrase();

        public static native boolean IsMnemonicWallet();

        public static native boolean IsMnemonicCorrect(String phrase);

        public static native boolean HaveUnconfirmedFunds();

        public static native long GetBalance();

        public static native void DoRescan();

        public static native UriRecipient IsValidRecipient(UriRecord request);

        public static native void performPaymentToRecipient(UriRecipient request);

        public static native ArrayList<TransactionRecord> getTransactionHistory();

        public static native TransactionRecord getTransaction(String txHash);

        public static native ArrayList<MutationRecord> getMutationHistory();

        public static native ArrayList<AddressRecord> getAddressBookRecords();

        public static native void addAddressBookRecord(AddressRecord address);

        public static native void deleteAddressBookRecord(AddressRecord address);

        public static native void PersistAndPruneForSPV();

        public static native void ResetUnifiedProgress();

        public static native ArrayList<PeerRecord> getPeers();

        public static native ArrayList<BlockInfoRecord> getLastSPVBlockInfos();

        public static native MonitorRecord getMonitoringStats();

        public static native void RegisterMonitorListener(GuldenMonitorListener listener);

        public static native void UnregisterMonitorListener(GuldenMonitorListener listener);
    }
}
