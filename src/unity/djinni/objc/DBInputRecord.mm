// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBInputRecord.h"


@implementation DBInputRecord

- (nonnull instancetype)initWithAddress:(nonnull NSString *)address
                                  label:(nonnull NSString *)label
                                   desc:(nonnull NSString *)desc
                                 isMine:(BOOL)isMine
{
    if (self = [super init]) {
        _address = [address copy];
        _label = [label copy];
        _desc = [desc copy];
        _isMine = isMine;
    }
    return self;
}

+ (nonnull instancetype)inputRecordWithAddress:(nonnull NSString *)address
                                         label:(nonnull NSString *)label
                                          desc:(nonnull NSString *)desc
                                        isMine:(BOOL)isMine
{
    return [(DBInputRecord*)[self alloc] initWithAddress:address
                                                   label:label
                                                    desc:desc
                                                  isMine:isMine];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p address:%@ label:%@ desc:%@ isMine:%@>", self.class, (void *)self, self.address, self.label, self.desc, @(self.isMine)];
}

@end
