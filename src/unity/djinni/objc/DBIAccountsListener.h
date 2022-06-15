// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBAccountRecord.h"
#import <Foundation/Foundation.h>


/** Interface to receive updates about accounts */
@protocol DBIAccountsListener

/** Notify that the active account has changed */
- (void)onActiveAccountChanged:(nonnull NSString *)accountUUID;

/** Notify that the active account name has changed */
- (void)onActiveAccountNameChanged:(nonnull NSString *)newAccountName;

/** Notify that an account name has changed */
- (void)onAccountNameChanged:(nonnull NSString *)accountUUID
              newAccountName:(nonnull NSString *)newAccountName;

/** Notify that a new account has been added */
- (void)onAccountAdded:(nonnull NSString *)accountUUID
           accountName:(nonnull NSString *)accountName;

/** Notify that an account has been deleted */
- (void)onAccountDeleted:(nonnull NSString *)accountUUID;

/** Notify that an account has been modified */
- (void)onAccountModified:(nonnull NSString *)accountUUID
              accountData:(nonnull DBAccountRecord *)accountData;

@end
