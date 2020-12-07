// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBWitnessEstimateInfoRecord.h"


@implementation DBWitnessEstimateInfoRecord

- (nonnull instancetype)initWithNetworkWeight:(int64_t)networkWeight
                                       weight:(int64_t)weight
                                        parts:(int64_t)parts
                  estimatedWitnessProbability:(double)estimatedWitnessProbability
                        estimatedBlocksPerDay:(double)estimatedBlocksPerDay
                       estimatedDailyEarnings:(int64_t)estimatedDailyEarnings
                    estimatedLifetimeEarnings:(int64_t)estimatedLifetimeEarnings
{
    if (self = [super init]) {
        _networkWeight = networkWeight;
        _weight = weight;
        _parts = parts;
        _estimatedWitnessProbability = estimatedWitnessProbability;
        _estimatedBlocksPerDay = estimatedBlocksPerDay;
        _estimatedDailyEarnings = estimatedDailyEarnings;
        _estimatedLifetimeEarnings = estimatedLifetimeEarnings;
    }
    return self;
}

+ (nonnull instancetype)witnessEstimateInfoRecordWithNetworkWeight:(int64_t)networkWeight
                                                            weight:(int64_t)weight
                                                             parts:(int64_t)parts
                                       estimatedWitnessProbability:(double)estimatedWitnessProbability
                                             estimatedBlocksPerDay:(double)estimatedBlocksPerDay
                                            estimatedDailyEarnings:(int64_t)estimatedDailyEarnings
                                         estimatedLifetimeEarnings:(int64_t)estimatedLifetimeEarnings
{
    return [(DBWitnessEstimateInfoRecord*)[self alloc] initWithNetworkWeight:networkWeight
                                                                      weight:weight
                                                                       parts:parts
                                                 estimatedWitnessProbability:estimatedWitnessProbability
                                                       estimatedBlocksPerDay:estimatedBlocksPerDay
                                                      estimatedDailyEarnings:estimatedDailyEarnings
                                                   estimatedLifetimeEarnings:estimatedLifetimeEarnings];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p networkWeight:%@ weight:%@ parts:%@ estimatedWitnessProbability:%@ estimatedBlocksPerDay:%@ estimatedDailyEarnings:%@ estimatedLifetimeEarnings:%@>", self.class, (void *)self, @(self.networkWeight), @(self.weight), @(self.parts), @(self.estimatedWitnessProbability), @(self.estimatedBlocksPerDay), @(self.estimatedDailyEarnings), @(self.estimatedLifetimeEarnings)];
}

@end
