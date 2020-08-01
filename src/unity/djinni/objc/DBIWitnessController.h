// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBWitnessEstimateInfoRecord.h"
#import "DBWitnessFundingResultRecord.h"
#import <Foundation/Foundation.h>


/** C++ interface to control witness accounts */
@interface DBIWitnessController : NSObject

/** Get information on min/max witness periods, weights etc. */
+ (nonnull NSDictionary<NSString *, NSString *> *)getNetworkLimits;

/** Get an estimate of weights/parts that a witness account will be funded with */
+ (nonnull DBWitnessEstimateInfoRecord *)getEstimatedWeight:(int64_t)amountToLock
                                         lockPeriodInBlocks:(int64_t)lockPeriodInBlocks;

/** Fund a witness account */
+ (nonnull DBWitnessFundingResultRecord *)fundWitnessAccount:(nonnull NSString *)fundingAccountUUID
                                          witnessAccountUUID:(nonnull NSString *)witnessAccountUUID
                                               fundingAmount:(int64_t)fundingAmount
                                 requestedLockPeriodInBlocks:(int64_t)requestedLockPeriodInBlocks;

@end
