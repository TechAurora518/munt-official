// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBAddressRecord.h"
#import "DBBlockInfoRecord.h"
#import "DBLegacyWalletResult.h"
#import "DBMonitorRecord.h"
#import "DBMutationRecord.h"
#import "DBPeerRecord.h"
#import "DBQrCodeRecord.h"
#import "DBTransactionRecord.h"
#import "DBUriRecipient.h"
#import "DBUriRecord.h"
#import <Foundation/Foundation.h>
@protocol DBGuldenMonitorListener;
@protocol DBGuldenUnifiedFrontend;

/** Interface constants */
extern int32_t const DBGuldenUnifiedBackendVersion;

/** This interface will be implemented in C++ and can be called from any language. */
@interface DBGuldenUnifiedBackend : NSObject

/**
 * Start the library
 * extraArgs - any additional commandline arguments as passed to GuldenD
 * NB!!! This call blocks until the library is terminated, it is the callers responsibility to place it inside a thread or similar.
 * If you are in an environment where this is not possible (node.js for example use InitUnityLibThreaded instead which places it in a thread on your behalf)
 */
+ (int32_t)InitUnityLib:(nonnull NSString *)dataDir
       staticFilterPath:(nonnull NSString *)staticFilterPath
     staticFilterOffset:(int64_t)staticFilterOffset
     staticFilterLength:(int64_t)staticFilterLength
                testnet:(BOOL)testnet
                signals:(nullable id<DBGuldenUnifiedFrontend>)signals
              extraArgs:(nonnull NSString *)extraArgs;

/** Threaded implementation of InitUnityLib */
+ (void)InitUnityLibThreaded:(nonnull NSString *)dataDir
            staticFilterPath:(nonnull NSString *)staticFilterPath
          staticFilterOffset:(int64_t)staticFilterOffset
          staticFilterLength:(int64_t)staticFilterLength
                     testnet:(BOOL)testnet
                     signals:(nullable id<DBGuldenUnifiedFrontend>)signals
                   extraArgs:(nonnull NSString *)extraArgs;

/** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
+ (BOOL)InitWalletFromRecoveryPhrase:(nonnull NSString *)phrase
                            password:(nonnull NSString *)password;

/** Continue creating wallet that was previously erased using EraseWalletSeedsAndAccounts */
+ (BOOL)ContinueWalletFromRecoveryPhrase:(nonnull NSString *)phrase
                                password:(nonnull NSString *)password;

/** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
+ (BOOL)InitWalletLinkedFromURI:(nonnull NSString *)linkedUri
                       password:(nonnull NSString *)password;

/** Continue creating wallet that was previously erased using EraseWalletSeedsAndAccounts */
+ (BOOL)ContinueWalletLinkedFromURI:(nonnull NSString *)linkedUri
                           password:(nonnull NSString *)password;

/** Create the wallet - this should only be called after receiving a `notifyInit...` signal from InitUnityLib */
+ (BOOL)InitWalletFromAndroidLegacyProtoWallet:(nonnull NSString *)walletFile
                                   oldPassword:(nonnull NSString *)oldPassword
                                   newPassword:(nonnull NSString *)newPassword;

/** Check if a file is a valid legacy proto wallet */
+ (DBLegacyWalletResult)isValidAndroidLegacyProtoWallet:(nonnull NSString *)walletFile
                                            oldPassword:(nonnull NSString *)oldPassword;

/** Check link URI for validity */
+ (BOOL)IsValidLinkURI:(nonnull NSString *)phrase;

/** Replace the existing wallet accounts with a new one from a linked URI - only after first emptying the wallet. */
+ (BOOL)ReplaceWalletLinkedFromURI:(nonnull NSString *)linkedUri
                          password:(nonnull NSString *)password;

/**
 * Erase the seeds and accounts of a wallet leaving an empty wallet (with things like the address book intact)
 * After calling this it will be necessary to create a new linked account or recovery phrase account again.
 * NB! This will empty a wallet regardless of whether it has funds in it or not and makes no provisions to check for this - it is the callers responsibility to ensure that erasing the wallet is safe to do in this regard.
 */
+ (BOOL)EraseWalletSeedsAndAccounts;

/**
 * Check recovery phrase for (syntactic) validity
 * Considered valid if the contained mnemonic is valid and the birth-number is either absent or passes Base-10 checksum
 */
+ (BOOL)IsValidRecoveryPhrase:(nonnull NSString *)phrase;

/** Generate a new recovery mnemonic */
+ (nonnull NSString *)GenerateRecoveryMnemonic;

/** Compute recovery phrase with birth number */
+ (nonnull NSString *)ComposeRecoveryPhrase:(nonnull NSString *)mnemonic
                                  birthTime:(int64_t)birthTime;

/** Stop the library */
+ (void)TerminateUnityLib;

/** Generate a QR code for a string, QR code will be as close to widthHint as possible when applying simple scaling. */
+ (nonnull DBQrCodeRecord *)QRImageFromString:(nonnull NSString *)qrString
                                    widthHint:(int32_t)widthHint;

/** Get a receive address from the wallet */
+ (nonnull NSString *)GetReceiveAddress;

/** Get the recovery phrase for the wallet */
+ (nonnull NSString *)GetRecoveryPhrase;

/** Check if the wallet is using a mnemonic seed ie. recovery phrase (else it is a linked wallet) */
+ (BOOL)IsMnemonicWallet;

/** Check if the phrase mnemonic is a correct one for the wallet (phrase can be with or without birth time) */
+ (BOOL)IsMnemonicCorrect:(nonnull NSString *)phrase;

/** Unlock wallet */
+ (BOOL)UnlockWallet:(nonnull NSString *)password;

/** Forcefully lock wallet again */
+ (BOOL)LockWallet;

/** Change the waller password */
+ (BOOL)ChangePassword:(nonnull NSString *)oldPassword
           newPassword:(nonnull NSString *)newPassword;

/** Check if the wallet has any transactions that are still pending confirmation, to be used to determine if e.g. it is safe to perform a link or whether we should wait. */
+ (BOOL)HaveUnconfirmedFunds;

/** Check current wallet balance (including unconfirmed funds) */
+ (int64_t)GetBalance;

/** Rescan blockchain for wallet transactions */
+ (void)DoRescan;

/** Check if text/address is something we are capable of sending money too */
+ (nonnull DBUriRecipient *)IsValidRecipient:(nonnull DBUriRecord *)request;

/** Attempt to pay a recipient, will throw on failure with description */
+ (void)performPaymentToRecipient:(nonnull DBUriRecipient *)request;

/** Get list of all transactions wallet has been involved in */
+ (nonnull NSArray<DBTransactionRecord *> *)getTransactionHistory;

/**
 * Get the wallet transaction for the hash
 * Will throw if not found
 */
+ (nonnull DBTransactionRecord *)getTransaction:(nonnull NSString *)txHash;

/** Get list of wallet mutations */
+ (nonnull NSArray<DBMutationRecord *> *)getMutationHistory;

/** Get list of all address book entries */
+ (nonnull NSArray<DBAddressRecord *> *)getAddressBookRecords;

/** Add a record to the address book */
+ (void)addAddressBookRecord:(nonnull DBAddressRecord *)address;

/** Delete a record from the address book */
+ (void)deleteAddressBookRecord:(nonnull DBAddressRecord *)address;

/** Interim persist and prune of state. Use at key moments like app backgrounding. */
+ (void)PersistAndPruneForSPV;

/**
 * Reset progress notification. In cases where there has been no progress for a long time, but the process
 * is still running the progress can be reset and will represent work to be done from this reset onwards.
 * For example when the process is in the background on iOS for a long long time (but has not been terminated
 * by the OS) this might make more sense then to continue the progress from where it was a day or more ago.
 */
+ (void)ResetUnifiedProgress;

/** Get connected peer info */
+ (nonnull NSArray<DBPeerRecord *> *)getPeers;

/** Get info of last blocks (at most 32) in SPV chain */
+ (nonnull NSArray<DBBlockInfoRecord *> *)getLastSPVBlockInfos;

+ (nonnull DBMonitorRecord *)getMonitoringStats;

+ (void)RegisterMonitorListener:(nullable id<DBGuldenMonitorListener>)listener;

+ (void)UnregisterMonitorListener:(nullable id<DBGuldenMonitorListener>)listener;

@end
