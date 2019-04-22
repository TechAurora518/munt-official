// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBOutputRecord.h"


@implementation DBOutputRecord

- (nonnull instancetype)initWithAmount:(int64_t)amount
                               address:(nonnull NSString *)address
                                 label:(nonnull NSString *)label
                                isMine:(BOOL)isMine
{
    if (self = [super init]) {
        _amount = amount;
        _address = [address copy];
        _label = [label copy];
        _isMine = isMine;
    }
    return self;
}

+ (nonnull instancetype)outputRecordWithAmount:(int64_t)amount
                                       address:(nonnull NSString *)address
                                         label:(nonnull NSString *)label
                                        isMine:(BOOL)isMine
{
    return [(DBOutputRecord*)[self alloc] initWithAmount:amount
                                                 address:address
                                                   label:label
                                                  isMine:isMine];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p amount:%@ address:%@ label:%@ isMine:%@>", self.class, (void *)self, @(self.amount), self.address, self.label, @(self.isMine)];
}

@end
