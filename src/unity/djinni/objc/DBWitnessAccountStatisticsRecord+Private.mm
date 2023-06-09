// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBWitnessAccountStatisticsRecord+Private.h"
#import "DJIMarshal+Private.h"
#include <cassert>

namespace djinni_generated {

auto WitnessAccountStatisticsRecord::toCpp(ObjcType obj) -> CppType
{
    assert(obj);
    return {::djinni::String::toCpp(obj.requestStatus),
            ::djinni::String::toCpp(obj.accountStatus),
            ::djinni::I64::toCpp(obj.blocksSinceLastActivity),
            ::djinni::I64::toCpp(obj.accountWeight),
            ::djinni::I64::toCpp(obj.accountWeightAtCreation),
            ::djinni::I64::toCpp(obj.accountParts),
            ::djinni::I64::toCpp(obj.accountAmountLocked),
            ::djinni::I64::toCpp(obj.accountAmountLockedAtCreation),
            ::djinni::I64::toCpp(obj.networkTipTotalWeight),
            ::djinni::I64::toCpp(obj.networkTotalWeightAtCreation),
            ::djinni::I64::toCpp(obj.accountInitialLockPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountRemainingLockPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountExpectedWitnessPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountEstimatedWitnessPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountInitialLockCreationBlockHeight),
            ::djinni::I32::toCpp(obj.compoundingPercent),
            ::djinni::Bool::toCpp(obj.isOptimal)};
}

auto WitnessAccountStatisticsRecord::fromCpp(const CppType& cpp) -> ObjcType
{
    return [[DBWitnessAccountStatisticsRecord alloc] initWithRequestStatus:(::djinni::String::fromCpp(cpp.request_status))
                                                             accountStatus:(::djinni::String::fromCpp(cpp.account_status))
                                                   blocksSinceLastActivity:(::djinni::I64::fromCpp(cpp.blocks_since_last_activity))
                                                             accountWeight:(::djinni::I64::fromCpp(cpp.account_weight))
                                                   accountWeightAtCreation:(::djinni::I64::fromCpp(cpp.account_weight_at_creation))
                                                              accountParts:(::djinni::I64::fromCpp(cpp.account_parts))
                                                       accountAmountLocked:(::djinni::I64::fromCpp(cpp.account_amount_locked))
                                             accountAmountLockedAtCreation:(::djinni::I64::fromCpp(cpp.account_amount_locked_at_creation))
                                                     networkTipTotalWeight:(::djinni::I64::fromCpp(cpp.network_tip_total_weight))
                                              networkTotalWeightAtCreation:(::djinni::I64::fromCpp(cpp.network_total_weight_at_creation))
                                          accountInitialLockPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_initial_lock_period_in_blocks))
                                        accountRemainingLockPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_remaining_lock_period_in_blocks))
                                      accountExpectedWitnessPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_expected_witness_period_in_blocks))
                                     accountEstimatedWitnessPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_estimated_witness_period_in_blocks))
                                     accountInitialLockCreationBlockHeight:(::djinni::I64::fromCpp(cpp.account_initial_lock_creation_block_height))
                                                        compoundingPercent:(::djinni::I32::fromCpp(cpp.compounding_percent))
                                                                 isOptimal:(::djinni::Bool::fromCpp(cpp.is_optimal))];
}

}  // namespace djinni_generated
