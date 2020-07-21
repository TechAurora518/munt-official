// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBAccountRecord.h"
#import "DBBalanceRecord.h"
#import "DBMutationRecord.h"
#import "DBTransactionRecord.h"
#import <Foundation/Foundation.h>
@protocol DBIAccountsListener;


/** C++ interface to control accounts */
@interface DBIAccountsController : NSObject

/** Register listener to be notified of account related events */
+ (void)setListener:(nullable id<DBIAccountsListener>)accountslistener;

/** Set the currently active account */
+ (BOOL)setActiveAccount:(nonnull NSString *)accountUUID;

/** Get the currently active account */
+ (nonnull NSString *)getActiveAccount;

/** Create an account, possible types are (HD/Mobile/Witness/Mining/Legacy). Returns the UUID of the new account */
+ (nonnull NSString *)createAccount:(nonnull NSString *)accountName
                        accountType:(nonnull NSString *)accountType;

/** Rename an account */
+ (BOOL)renameAccount:(nonnull NSString *)accountUUID
       newAccountName:(nonnull NSString *)newAccountName;

/** Get a URI that will enable 'linking' of this account in another wallet (for e.g. mobile wallet linking) for an account. Empty on failiure.  */
+ (nonnull NSString *)getAccountLinkURI:(nonnull NSString *)accountUUID;

/** Get a URI that will enable creation of a "witness only" account in another wallet that can witness on behalf of this account */
+ (nonnull NSString *)getWitnessKeyURI:(nonnull NSString *)accountUUID;

/**
 * Create a new "witness-only" account from a previously exported URI
 * Returns UUID on success, empty string on failiure
 */
+ (nonnull NSString *)createAccountFromWitnessKeyURI:(nonnull NSString *)witnessKeyURI
                                      newAccountName:(nonnull NSString *)newAccountName;

/** Delete an account, account remains available in background but is hidden from user */
+ (BOOL)deleteAccount:(nonnull NSString *)accountUUID;

/**
 * Purge an account, account is permenently removed from wallet (but may still reappear in some instances if it is an HD account and user recovers from phrase in future)
 * If it is a Legacy or imported witness key or similar account then it will be gone forever
 * Generally prefer 'deleteAccount' and use this with caution
 */
+ (BOOL)purgeAccount:(nonnull NSString *)accountUUID;

/** List all currently visible accounts in the wallet */
+ (nonnull NSArray<DBAccountRecord *> *)listAccounts;

/** Check balance for active account */
+ (nonnull DBBalanceRecord *)getActiveAccountBalance;

/** Check balance for account */
+ (nonnull DBBalanceRecord *)getAccountBalance:(nonnull NSString *)accountUUID;

/** Check balance for all accounts, returns a map of account_uuid->balance_record */
+ (nonnull NSDictionary<NSString *, DBBalanceRecord *> *)getAllAccountBalances;

/** Get list of all transactions account has been involved in */
+ (nonnull NSArray<DBTransactionRecord *> *)getTransactionHistory:(nonnull NSString *)accountUUID;

/** Get list of mutations for account */
+ (nonnull NSArray<DBMutationRecord *> *)getMutationHistory:(nonnull NSString *)accountUUID;

@end
