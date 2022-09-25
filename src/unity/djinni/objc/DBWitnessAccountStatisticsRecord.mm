// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBWitnessAccountStatisticsRecord.h"


@implementation DBWitnessAccountStatisticsRecord

- (nonnull instancetype)initWithRequestStatus:(nonnull NSString *)requestStatus
                                accountStatus:(nonnull NSString *)accountStatus
                                accountWeight:(int64_t)accountWeight
                      accountWeightAtCreation:(int64_t)accountWeightAtCreation
                                 accountParts:(int64_t)accountParts
                          accountAmountLocked:(int64_t)accountAmountLocked
                accountAmountLockedAtCreation:(int64_t)accountAmountLockedAtCreation
                        networkTipTotalWeight:(int64_t)networkTipTotalWeight
                 networkTotalWeightAtCreation:(int64_t)networkTotalWeightAtCreation
             accountInitialLockPeriodInBlocks:(int64_t)accountInitialLockPeriodInBlocks
           accountRemainingLockPeriodInBlocks:(int64_t)accountRemainingLockPeriodInBlocks
         accountExpectedWitnessPeriodInBlocks:(int64_t)accountExpectedWitnessPeriodInBlocks
        accountEstimatedWitnessPeriodInBlocks:(int64_t)accountEstimatedWitnessPeriodInBlocks
        accountInitialLockCreationBlockHeight:(int64_t)accountInitialLockCreationBlockHeight
                           compoundingPercent:(int32_t)compoundingPercent
                                    isOptimal:(BOOL)isOptimal
{
    if (self = [super init]) {
        _requestStatus = [requestStatus copy];
        _accountStatus = [accountStatus copy];
        _accountWeight = accountWeight;
        _accountWeightAtCreation = accountWeightAtCreation;
        _accountParts = accountParts;
        _accountAmountLocked = accountAmountLocked;
        _accountAmountLockedAtCreation = accountAmountLockedAtCreation;
        _networkTipTotalWeight = networkTipTotalWeight;
        _networkTotalWeightAtCreation = networkTotalWeightAtCreation;
        _accountInitialLockPeriodInBlocks = accountInitialLockPeriodInBlocks;
        _accountRemainingLockPeriodInBlocks = accountRemainingLockPeriodInBlocks;
        _accountExpectedWitnessPeriodInBlocks = accountExpectedWitnessPeriodInBlocks;
        _accountEstimatedWitnessPeriodInBlocks = accountEstimatedWitnessPeriodInBlocks;
        _accountInitialLockCreationBlockHeight = accountInitialLockCreationBlockHeight;
        _compoundingPercent = compoundingPercent;
        _isOptimal = isOptimal;
    }
    return self;
}

+ (nonnull instancetype)witnessAccountStatisticsRecordWithRequestStatus:(nonnull NSString *)requestStatus
                                                          accountStatus:(nonnull NSString *)accountStatus
                                                          accountWeight:(int64_t)accountWeight
                                                accountWeightAtCreation:(int64_t)accountWeightAtCreation
                                                           accountParts:(int64_t)accountParts
                                                    accountAmountLocked:(int64_t)accountAmountLocked
                                          accountAmountLockedAtCreation:(int64_t)accountAmountLockedAtCreation
                                                  networkTipTotalWeight:(int64_t)networkTipTotalWeight
                                           networkTotalWeightAtCreation:(int64_t)networkTotalWeightAtCreation
                                       accountInitialLockPeriodInBlocks:(int64_t)accountInitialLockPeriodInBlocks
                                     accountRemainingLockPeriodInBlocks:(int64_t)accountRemainingLockPeriodInBlocks
                                   accountExpectedWitnessPeriodInBlocks:(int64_t)accountExpectedWitnessPeriodInBlocks
                                  accountEstimatedWitnessPeriodInBlocks:(int64_t)accountEstimatedWitnessPeriodInBlocks
                                  accountInitialLockCreationBlockHeight:(int64_t)accountInitialLockCreationBlockHeight
                                                     compoundingPercent:(int32_t)compoundingPercent
                                                              isOptimal:(BOOL)isOptimal
{
    return [(DBWitnessAccountStatisticsRecord*)[self alloc] initWithRequestStatus:requestStatus
                                                                    accountStatus:accountStatus
                                                                    accountWeight:accountWeight
                                                          accountWeightAtCreation:accountWeightAtCreation
                                                                     accountParts:accountParts
                                                              accountAmountLocked:accountAmountLocked
                                                    accountAmountLockedAtCreation:accountAmountLockedAtCreation
                                                            networkTipTotalWeight:networkTipTotalWeight
                                                     networkTotalWeightAtCreation:networkTotalWeightAtCreation
                                                 accountInitialLockPeriodInBlocks:accountInitialLockPeriodInBlocks
                                               accountRemainingLockPeriodInBlocks:accountRemainingLockPeriodInBlocks
                                             accountExpectedWitnessPeriodInBlocks:accountExpectedWitnessPeriodInBlocks
                                            accountEstimatedWitnessPeriodInBlocks:accountEstimatedWitnessPeriodInBlocks
                                            accountInitialLockCreationBlockHeight:accountInitialLockCreationBlockHeight
                                                               compoundingPercent:compoundingPercent
                                                                        isOptimal:isOptimal];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p requestStatus:%@ accountStatus:%@ accountWeight:%@ accountWeightAtCreation:%@ accountParts:%@ accountAmountLocked:%@ accountAmountLockedAtCreation:%@ networkTipTotalWeight:%@ networkTotalWeightAtCreation:%@ accountInitialLockPeriodInBlocks:%@ accountRemainingLockPeriodInBlocks:%@ accountExpectedWitnessPeriodInBlocks:%@ accountEstimatedWitnessPeriodInBlocks:%@ accountInitialLockCreationBlockHeight:%@ compoundingPercent:%@ isOptimal:%@>", self.class, (void *)self, self.requestStatus, self.accountStatus, @(self.accountWeight), @(self.accountWeightAtCreation), @(self.accountParts), @(self.accountAmountLocked), @(self.accountAmountLockedAtCreation), @(self.networkTipTotalWeight), @(self.networkTotalWeightAtCreation), @(self.accountInitialLockPeriodInBlocks), @(self.accountRemainingLockPeriodInBlocks), @(self.accountExpectedWitnessPeriodInBlocks), @(self.accountEstimatedWitnessPeriodInBlocks), @(self.accountInitialLockCreationBlockHeight), @(self.compoundingPercent), @(self.isOptimal)];
}

@end
