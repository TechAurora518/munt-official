// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBOutputRecord.h"


@implementation DBOutputRecord

- (nonnull instancetype)initWithAmount:(int64_t)amount
                               address:(nonnull NSString *)address
                                 label:(nonnull NSString *)label
{
    if (self = [super init]) {
        _amount = amount;
        _address = [address copy];
        _label = [label copy];
    }
    return self;
}

+ (nonnull instancetype)outputRecordWithAmount:(int64_t)amount
                                       address:(nonnull NSString *)address
                                         label:(nonnull NSString *)label
{
    return [[self alloc] initWithAmount:amount
                                address:address
                                  label:label];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p amount:%@ address:%@ label:%@>", self.class, (void *)self, @(self.amount), self.address, self.label];
}

@end
