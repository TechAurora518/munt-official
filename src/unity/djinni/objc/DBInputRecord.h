// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import <Foundation/Foundation.h>

@interface DBInputRecord : NSObject
- (nonnull instancetype)initWithAddress:(nonnull NSString *)address
                                  label:(nonnull NSString *)label
                                   desc:(nonnull NSString *)desc
                                 isMine:(BOOL)isMine;
+ (nonnull instancetype)inputRecordWithAddress:(nonnull NSString *)address
                                         label:(nonnull NSString *)label
                                          desc:(nonnull NSString *)desc
                                        isMine:(BOOL)isMine;

@property (nonatomic, readonly, nonnull) NSString * address;

@property (nonatomic, readonly, nonnull) NSString * label;

@property (nonatomic, readonly, nonnull) NSString * desc;

@property (nonatomic, readonly) BOOL isMine;

@end
