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
            ::djinni::I64::toCpp(obj.accountWeight),
            ::djinni::I64::toCpp(obj.accountAmountLocked),
            ::djinni::I64::toCpp(obj.accountWeightAtCreation),
            ::djinni::I64::toCpp(obj.networkTipTotalWeight),
            ::djinni::I64::toCpp(obj.networkTotalWeightAtAccountCreationTime),
            ::djinni::I64::toCpp(obj.accountInitialLockPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountRemainingLockPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountExpectedWitnessPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountEstimatedWitnessPeriodInBlocks),
            ::djinni::I64::toCpp(obj.accountInitialLockCreationBlockHeight),
            ::djinni::Bool::toCpp(obj.accountIsCompounding)};
}

auto WitnessAccountStatisticsRecord::fromCpp(const CppType& cpp) -> ObjcType
{
    return [[DBWitnessAccountStatisticsRecord alloc] initWithRequestStatus:(::djinni::String::fromCpp(cpp.request_status))
                                                             accountStatus:(::djinni::String::fromCpp(cpp.account_status))
                                                             accountWeight:(::djinni::I64::fromCpp(cpp.account_weight))
                                                       accountAmountLocked:(::djinni::I64::fromCpp(cpp.account_amount_locked))
                                                   accountWeightAtCreation:(::djinni::I64::fromCpp(cpp.account_weight_at_creation))
                                                     networkTipTotalWeight:(::djinni::I64::fromCpp(cpp.network_tip_total_weight))
                                   networkTotalWeightAtAccountCreationTime:(::djinni::I64::fromCpp(cpp.network_total_weight_at_account_creation_time))
                                          accountInitialLockPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_initial_lock_period_in_blocks))
                                        accountRemainingLockPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_remaining_lock_period_in_blocks))
                                      accountExpectedWitnessPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_expected_witness_period_in_blocks))
                                     accountEstimatedWitnessPeriodInBlocks:(::djinni::I64::fromCpp(cpp.account_estimated_witness_period_in_blocks))
                                     accountInitialLockCreationBlockHeight:(::djinni::I64::fromCpp(cpp.account_initial_lock_creation_block_height))
                                                      accountIsCompounding:(::djinni::Bool::fromCpp(cpp.account_is_compounding))];
}

}  // namespace djinni_generated
