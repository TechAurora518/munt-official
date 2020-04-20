// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import <Foundation/Foundation.h>

@interface DBOutputRecord : NSObject
- (nonnull instancetype)initWithAmount:(int64_t)amount
                               address:(nonnull NSString *)address
                                 label:(nonnull NSString *)label
                                  desc:(nonnull NSString *)desc
                                isMine:(BOOL)isMine;
+ (nonnull instancetype)outputRecordWithAmount:(int64_t)amount
                                       address:(nonnull NSString *)address
                                         label:(nonnull NSString *)label
                                          desc:(nonnull NSString *)desc
                                        isMine:(BOOL)isMine;

@property (nonatomic, readonly) int64_t amount;

@property (nonatomic, readonly, nonnull) NSString * address;

@property (nonatomic, readonly, nonnull) NSString * label;

@property (nonatomic, readonly, nonnull) NSString * desc;

@property (nonatomic, readonly) BOOL isMine;

@end
