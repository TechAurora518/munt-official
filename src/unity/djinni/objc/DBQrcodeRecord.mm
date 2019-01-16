// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBQrcodeRecord.h"


@implementation DBQrcodeRecord

- (nonnull instancetype)initWithWidth:(int32_t)width
                            pixelData:(nonnull NSData *)pixelData
{
    if (self = [super init]) {
        _width = width;
        _pixelData = [pixelData copy];
    }
    return self;
}

+ (nonnull instancetype)qrcodeRecordWithWidth:(int32_t)width
                                    pixelData:(nonnull NSData *)pixelData
{
    return [[self alloc] initWithWidth:width
                             pixelData:pixelData];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p width:%@ pixelData:%@>", self.class, (void *)self, @(self.width), self.pixelData];
}

@end
