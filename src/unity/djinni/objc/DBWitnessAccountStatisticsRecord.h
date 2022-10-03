// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import <Foundation/Foundation.h>

@interface DBWitnessAccountStatisticsRecord : NSObject
- (nonnull instancetype)initWithRequestStatus:(nonnull NSString *)requestStatus
                                accountStatus:(nonnull NSString *)accountStatus
                      blocksSinceLastActivity:(int64_t)blocksSinceLastActivity
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
                                    isOptimal:(BOOL)isOptimal;
+ (nonnull instancetype)witnessAccountStatisticsRecordWithRequestStatus:(nonnull NSString *)requestStatus
                                                          accountStatus:(nonnull NSString *)accountStatus
                                                blocksSinceLastActivity:(int64_t)blocksSinceLastActivity
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
                                                              isOptimal:(BOOL)isOptimal;

/** Success if request succeeded, otherwise an error message */
@property (nonatomic, readonly, nonnull) NSString * requestStatus;

/** Current state of the witness account, one of: "empty", "empty_with_remainder", "pending", "witnessing", "ended", "expired", "emptying" */
@property (nonatomic, readonly, nonnull) NSString * accountStatus;

/** Account weight */
@property (nonatomic, readonly) int64_t blocksSinceLastActivity;

/** Account weight */
@property (nonatomic, readonly) int64_t accountWeight;

/** Account weight when it was created */
@property (nonatomic, readonly) int64_t accountWeightAtCreation;

/** How many parts the account weight is split up into */
@property (nonatomic, readonly) int64_t accountParts;

/** Account amount currently locked */
@property (nonatomic, readonly) int64_t accountAmountLocked;

/** Account amount locked when it was created */
@property (nonatomic, readonly) int64_t accountAmountLockedAtCreation;

/** Current network weight */
@property (nonatomic, readonly) int64_t networkTipTotalWeight;

/** Network weight when account was created */
@property (nonatomic, readonly) int64_t networkTotalWeightAtCreation;

/** Account total lock period in blocks (from creation block) */
@property (nonatomic, readonly) int64_t accountInitialLockPeriodInBlocks;

/** Account remaining lock period in blocks (from chain tip) */
@property (nonatomic, readonly) int64_t accountRemainingLockPeriodInBlocks;

/** How often the account is "expected" by the network to witness in order to not be kicked off */
@property (nonatomic, readonly) int64_t accountExpectedWitnessPeriodInBlocks;

/** How often the account is estimated to witness */
@property (nonatomic, readonly) int64_t accountEstimatedWitnessPeriodInBlocks;

/** Height at which the account lock first entered the chain */
@property (nonatomic, readonly) int64_t accountInitialLockCreationBlockHeight;

/** How much of the reward that this account earns is set to be compound */
@property (nonatomic, readonly) int32_t compoundingPercent;

/** Is the account weight split in an optimal way */
@property (nonatomic, readonly) BOOL isOptimal;

@end
