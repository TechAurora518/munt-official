// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class GuldenMonitorListener;
class GuldenUnifiedFrontend;
struct AddressRecord;
struct BlockinfoRecord;
struct MonitorRecord;
struct MutationRecord;
struct PeerRecord;
struct QrcodeRecord;
struct TransactionRecord;
struct UriRecipient;
struct UriRecord;

/** This interface will be implemented in C++ and can be called from any language. */
class GuldenUnifiedBackend {
public:
    virtual ~GuldenUnifiedBackend() {}

    /** Interface constants */
    static constexpr int32_t VERSION = 1;

    /** Start the library */
    static int32_t InitUnityLib(const std::string & data_dir, bool testnet, const std::shared_ptr<GuldenUnifiedFrontend> & signals);

    /** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
    static bool InitWalletFromRecoveryPhrase(const std::string & phrase);

    /** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
    static bool InitWalletLinkedFromURI(const std::string & linked_uri);

    /** Replace the existing wallet accounts with a new one from a linked URI - only after first emptying the wallet. */
    static bool ReplaceWalletLinkedFromURI(const std::string & linked_uri);

    /** Check link URI for validity */
    static bool IsValidLinkURI(const std::string & phrase);

    /**
     * Check recovery phrase for (syntactic) validity
     * Considered valid if the contained mnemonic is valid and the birthnumber is either absent or passes Base-10 checksum
     */
    static bool IsValidRecoveryPhrase(const std::string & phrase);

    /** Generate a new recovery mnemonic */
    static std::string GenerateRecoveryMnemonic();

    /** Stop the library */
    static void TerminateUnityLib();

    /** Generate a QR code for a string, QR code will be as close to width_hint as possible when applying simple scaling. */
    static QrcodeRecord QRImageFromString(const std::string & qr_string, int32_t width_hint);

    /** Get a receive address from the wallet */
    static std::string GetReceiveAddress();

    /** Get the recovery phrase for the wallet */
    static std::string GetRecoveryPhrase();

    /** Check if the wallet has any transactions that are still pending confirmation, to be used to determine if e.g. it is safe to perform a link or whether we should wait. */
    static bool HaveUnconfirmedFunds();

    /** Check current wallet balance (including unconfirmed funds) */
    static int64_t GetBalance();

    /** Rescan blockchain for wallet transactions */
    static void DoRescan();

    /** Check if text/address is something we are capable of sending money too */
    static UriRecipient IsValidRecipient(const UriRecord & request);

    /** Attempt to pay a recipient, will throw on failure with descriptiopn */
    static void performPaymentToRecipient(const UriRecipient & request);

    /** Get list of all transactions wallet has been involved in */
    static std::vector<TransactionRecord> getTransactionHistory();

    /**
     * Get the wallet transaction for the hash
     * Will throw if not found
     */
    static TransactionRecord getTransaction(const std::string & txHash);

    /** Get list of wallet mutations */
    static std::vector<MutationRecord> getMutationHistory();

    /** Get list of all address book entries */
    static std::vector<AddressRecord> getAddressBookRecords();

    /** Add a record to the address book */
    static void addAddressBookRecord(const AddressRecord & address);

    /** Delete a record from the address book */
    static void deleteAddressBookRecord(const AddressRecord & address);

    /** Interim persist and prune of state. Use at key moments like app backgrounding. */
    static void PersistAndPruneForSPV();

    /**
     * Reset progress notification. In cases where there has been no progress for a long time, but the process
     * is still running the progress can be reset and will represent work to be done from this reset onwards.
     * For example when the process is in the background on iOS for a long long time (but has not been terminated
     * by the OS) this might make more sense then to continue the progress from where it was a day or more ago.
     */
    static void ResetUnifiedProgress();

    /** Get connected peer info */
    static std::vector<PeerRecord> getPeers();

    /** Get info of last blocks (at most 32) in SPV chain */
    static std::vector<BlockinfoRecord> getLastSPVBlockinfos();

    static MonitorRecord getMonitoringStats();

    static void RegisterMonitorListener(const std::shared_ptr<GuldenMonitorListener> & listener);

    static void UnregisterMonitorListener(const std::shared_ptr<GuldenMonitorListener> & listener);
};
