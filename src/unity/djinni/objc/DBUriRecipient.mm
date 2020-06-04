// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBUriRecipient.h"


@implementation DBUriRecipient

- (nonnull instancetype)initWithValid:(BOOL)valid
                              address:(nonnull NSString *)address
                                label:(nonnull NSString *)label
                                 desc:(nonnull NSString *)desc
                               amount:(int64_t)amount
{
    if (self = [super init]) {
        _valid = valid;
        _address = [address copy];
        _label = [label copy];
        _desc = [desc copy];
        _amount = amount;
    }
    return self;
}

+ (nonnull instancetype)uriRecipientWithValid:(BOOL)valid
                                      address:(nonnull NSString *)address
                                        label:(nonnull NSString *)label
                                         desc:(nonnull NSString *)desc
                                       amount:(int64_t)amount
{
    return [(DBUriRecipient*)[self alloc] initWithValid:valid
                                                address:address
                                                  label:label
                                                   desc:desc
                                                 amount:amount];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p valid:%@ address:%@ label:%@ desc:%@ amount:%@>", self.class, (void *)self, @(self.valid), self.address, self.label, self.desc, @(self.amount)];
}

@end
