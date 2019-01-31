// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import <Foundation/Foundation.h>

@interface DBMutationRecord : NSObject
- (nonnull instancetype)initWithChange:(int64_t)change
                             timestamp:(int64_t)timestamp
                                txHash:(nonnull NSString *)txHash;
+ (nonnull instancetype)mutationRecordWithChange:(int64_t)change
                                       timestamp:(int64_t)timestamp
                                          txHash:(nonnull NSString *)txHash;

@property (nonatomic, readonly) int64_t change;

@property (nonatomic, readonly) int64_t timestamp;

@property (nonatomic, readonly, nonnull) NSString * txHash;

@end
