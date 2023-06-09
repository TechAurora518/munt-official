// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBAccountLinkRecord.h"
#import <Foundation/Foundation.h>

@interface DBAccountRecord : NSObject
- (nonnull instancetype)initWithUUID:(nonnull NSString *)UUID
                               label:(nonnull NSString *)label
                               state:(nonnull NSString *)state
                                type:(nonnull NSString *)type
                                isHD:(BOOL)isHD
                        accountLinks:(nonnull NSArray<DBAccountLinkRecord *> *)accountLinks;
+ (nonnull instancetype)accountRecordWithUUID:(nonnull NSString *)UUID
                                        label:(nonnull NSString *)label
                                        state:(nonnull NSString *)state
                                         type:(nonnull NSString *)type
                                         isHD:(BOOL)isHD
                                 accountLinks:(nonnull NSArray<DBAccountLinkRecord *> *)accountLinks;

@property (nonatomic, readonly, nonnull) NSString * UUID;

@property (nonatomic, readonly, nonnull) NSString * label;

@property (nonatomic, readonly, nonnull) NSString * state;

@property (nonatomic, readonly, nonnull) NSString * type;

/**Is this account 'HD' (i.e. part of what can be recovered from a recovery phrase) */
@property (nonatomic, readonly) BOOL isHD;

/**Has this account been linked to any other services/wallets; if so which see 'account_link_record' for more information */
@property (nonatomic, readonly, nonnull) NSArray<DBAccountLinkRecord *> * accountLinks;

@end
