// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class GuldenUnifiedFrontend;
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
    static int32_t InitUnityLib(const std::string & data_dir, const std::shared_ptr<GuldenUnifiedFrontend> & signals);

    /** Stop the library */
    static void TerminateUnityLib();

    /** Generate a QR code for a string, QR code will be as close to width_hint as possible when applying simple scaling. */
    static QrcodeRecord QRImageFromString(const std::string & qr_string, int32_t width_hint);

    /** Get a receive address from the wallet */
    static std::string GetReceiveAddress();

    /** Get a receive address from the wallet */
    static std::string GetRecoveryPhrase();

    /** Check if text/address is something we are capable of sending money too */
    static UriRecipient IsValidRecipient(const UriRecord & request);

    /** Attempt to pay a recipient */
    static bool performPaymentToRecipient(const UriRecipient & request);

    /** Get list of all transactions wallet has been involved in */
    static std::vector<TransactionRecord> getTransactionHistory();
};
