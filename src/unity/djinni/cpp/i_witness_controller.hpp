// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

struct WitnessAccountStatisticsRecord;
struct WitnessEstimateInfoRecord;
struct WitnessFundingResultRecord;

/** C++ interface to control witness accounts */
class IWitnessController {
public:
    virtual ~IWitnessController() {}

    /** Get information on min/max witness periods, weights etc. */
    static std::unordered_map<std::string, std::string> getNetworkLimits();

    /** Get an estimate of weights/parts that a witness account will be funded with */
    static WitnessEstimateInfoRecord getEstimatedWeight(int64_t amount_to_lock, int64_t lock_period_in_blocks);

    /** Fund a witness account */
    static WitnessFundingResultRecord fundWitnessAccount(const std::string & funding_account_UUID, const std::string & witness_account_UUID, int64_t funding_amount, int64_t requestedLockPeriodInBlocks);

    /** Get information on account weight and other witness statistics for account */
    static WitnessAccountStatisticsRecord getAccountWitnessStatistics(const std::string & witnessAccountUUID);
};
