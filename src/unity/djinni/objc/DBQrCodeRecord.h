// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import <Foundation/Foundation.h>

/**
 * Copyright (c) 2020 The Gulden developers
 * Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
 * Distributed under the GULDEN software license, see the accompanying
 * file COPYING
 */
@interface DBQrCodeRecord : NSObject
- (nonnull instancetype)initWithWidth:(int32_t)width
                            pixelData:(nonnull NSData *)pixelData;
+ (nonnull instancetype)qrCodeRecordWithWidth:(int32_t)width
                                    pixelData:(nonnull NSData *)pixelData;

@property (nonatomic, readonly) int32_t width;

@property (nonatomic, readonly, nonnull) NSData * pixelData;

@end
