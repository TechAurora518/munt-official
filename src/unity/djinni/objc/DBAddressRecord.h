// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import <Foundation/Foundation.h>

@interface DBAddressRecord : NSObject
- (nonnull instancetype)initWithAddress:(nonnull NSString *)address
                                purpose:(nonnull NSString *)purpose
                                   name:(nonnull NSString *)name;
+ (nonnull instancetype)addressRecordWithAddress:(nonnull NSString *)address
                                         purpose:(nonnull NSString *)purpose
                                            name:(nonnull NSString *)name;

@property (nonatomic, readonly, nonnull) NSString * address;

@property (nonatomic, readonly, nonnull) NSString * purpose;

@property (nonatomic, readonly, nonnull) NSString * name;

@end
