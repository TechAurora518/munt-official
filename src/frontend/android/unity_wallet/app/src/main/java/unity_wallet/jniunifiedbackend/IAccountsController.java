// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package unity_wallet.jniunifiedbackend;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.atomic.AtomicBoolean;

/** C++ interface to control accounts */
public abstract class IAccountsController {
    /** Register listener to be notified of account related events */
    public static void setListener(IAccountsListener accountslistener)
    {
        CppProxy.setListener(accountslistener);
    }

    /** List all currently visible accounts in the wallet */
    public static ArrayList<AccountRecord> listAccounts()
    {
        return CppProxy.listAccounts();
    }

    /** Set the currently active account */
    public static boolean setActiveAccount(String accountUUID)
    {
        return CppProxy.setActiveAccount(accountUUID);
    }

    /** Get the currently active account */
    public static String getActiveAccount()
    {
        return CppProxy.getActiveAccount();
    }

    /** Create an account, possible types are (HD/Mobile/Witness/Mining/Legacy). Returns the UUID of the new account */
    public static String createAccount(String accountName, String accountType)
    {
        return CppProxy.createAccount(accountName,
                                      accountType);
    }

    /** Check name of account */
    public static String getAccountName(String accountUUID)
    {
        return CppProxy.getAccountName(accountUUID);
    }

    /** Rename an account */
    public static boolean renameAccount(String accountUUID, String newAccountName)
    {
        return CppProxy.renameAccount(accountUUID,
                                      newAccountName);
    }

    /** Delete an account, account remains available in background but is hidden from user */
    public static boolean deleteAccount(String accountUUID)
    {
        return CppProxy.deleteAccount(accountUUID);
    }

    /**
     * Purge an account, account is permenently removed from wallet (but may still reappear in some instances if it is an HD account and user recovers from phrase in future)
     * If it is a Legacy or imported witness key or similar account then it will be gone forever
     * Generally prefer 'deleteAccount' and use this with caution
     */
    public static boolean purgeAccount(String accountUUID)
    {
        return CppProxy.purgeAccount(accountUUID);
    }

    /** Get a URI that will enable 'linking' of this account in another wallet (for e.g. mobile wallet linking) for an account. Empty on failiure.  */
    public static String getAccountLinkURI(String accountUUID)
    {
        return CppProxy.getAccountLinkURI(accountUUID);
    }

    /** Get a URI that will enable creation of a "witness only" account in another wallet that can witness on behalf of this account */
    public static String getWitnessKeyURI(String accountUUID)
    {
        return CppProxy.getWitnessKeyURI(accountUUID);
    }

    /**
     * Create a new "witness-only" account from a previously exported URI
     * Returns UUID on success, empty string on failiure
     */
    public static String createAccountFromWitnessKeyURI(String witnessKeyURI, String newAccountName)
    {
        return CppProxy.createAccountFromWitnessKeyURI(witnessKeyURI,
                                                       newAccountName);
    }

    /** Get a receive address for account */
    public static String getReceiveAddress(String accountUUID)
    {
        return CppProxy.getReceiveAddress(accountUUID);
    }

    /** Get list of all transactions account has been involved in */
    public static ArrayList<TransactionRecord> getTransactionHistory(String accountUUID)
    {
        return CppProxy.getTransactionHistory(accountUUID);
    }

    /** Get list of mutations for account */
    public static ArrayList<MutationRecord> getMutationHistory(String accountUUID)
    {
        return CppProxy.getMutationHistory(accountUUID);
    }

    /** Check balance for active account */
    public static BalanceRecord getActiveAccountBalance()
    {
        return CppProxy.getActiveAccountBalance();
    }

    /** Check balance for account */
    public static BalanceRecord getAccountBalance(String accountUUID)
    {
        return CppProxy.getAccountBalance(accountUUID);
    }

    /** Check balance for all accounts, returns a map of account_uuid->balance_record */
    public static HashMap<String, BalanceRecord> getAllAccountBalances()
    {
        return CppProxy.getAllAccountBalances();
    }

    /**Register with wallet that this account has been "linked" with an external service (e.g. to host holding key) */
    public static boolean addAccountLink(String accountUUID, String serviceName, String data)
    {
        return CppProxy.addAccountLink(accountUUID,
                                       serviceName,
                                       data);
    }

    /**Register with wallet to remove an existing link */
    public static boolean removeAccountLink(String accountUUID, String serviceName)
    {
        return CppProxy.removeAccountLink(accountUUID,
                                          serviceName);
    }

    /**List all active account links that we have previously registered */
    public static ArrayList<AccountLinkRecord> listAccountLinks(String accountUUID)
    {
        return CppProxy.listAccountLinks(accountUUID);
    }

    private static final class CppProxy extends IAccountsController
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

        public static native void setListener(IAccountsListener accountslistener);

        public static native ArrayList<AccountRecord> listAccounts();

        public static native boolean setActiveAccount(String accountUUID);

        public static native String getActiveAccount();

        public static native String createAccount(String accountName, String accountType);

        public static native String getAccountName(String accountUUID);

        public static native boolean renameAccount(String accountUUID, String newAccountName);

        public static native boolean deleteAccount(String accountUUID);

        public static native boolean purgeAccount(String accountUUID);

        public static native String getAccountLinkURI(String accountUUID);

        public static native String getWitnessKeyURI(String accountUUID);

        public static native String createAccountFromWitnessKeyURI(String witnessKeyURI, String newAccountName);

        public static native String getReceiveAddress(String accountUUID);

        public static native ArrayList<TransactionRecord> getTransactionHistory(String accountUUID);

        public static native ArrayList<MutationRecord> getMutationHistory(String accountUUID);

        public static native BalanceRecord getActiveAccountBalance();

        public static native BalanceRecord getAccountBalance(String accountUUID);

        public static native HashMap<String, BalanceRecord> getAllAccountBalances();

        public static native boolean addAccountLink(String accountUUID, String serviceName, String data);

        public static native boolean removeAccountLink(String accountUUID, String serviceName);

        public static native ArrayList<AccountLinkRecord> listAccountLinks(String accountUUID);
    }
}
