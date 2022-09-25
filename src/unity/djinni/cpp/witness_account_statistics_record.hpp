// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <string>
#include <utility>

struct WitnessAccountStatisticsRecord final {
    /** Success if request succeeded, otherwise an error message */
    std::string request_status;
    /** Current state of the witness account, one of: "empty", "empty_with_remainder", "pending", "witnessing", "ended", "expired", "emptying" */
    std::string account_status;
    /** Account weight */
    int64_t account_weight;
    /** Account weight when it was created */
    int64_t account_weight_at_creation;
    /** How many parts the account weight is split up into */
    int64_t account_parts;
    /** Account amount currently locked */
    int64_t account_amount_locked;
    /** Account amount locked when it was created */
    int64_t account_amount_locked_at_creation;
    /** Current network weight */
    int64_t network_tip_total_weight;
    /** Network weight when account was created */
    int64_t network_total_weight_at_creation;
    /** Account total lock period in blocks (from creation block) */
    int64_t account_initial_lock_period_in_blocks;
    /** Account remaining lock period in blocks (from chain tip) */
    int64_t account_remaining_lock_period_in_blocks;
    /** How often the account is "expected" by the network to witness in order to not be kicked off */
    int64_t account_expected_witness_period_in_blocks;
    /** How often the account is estimated to witness */
    int64_t account_estimated_witness_period_in_blocks;
    /** Height at which the account lock first entered the chain */
    int64_t account_initial_lock_creation_block_height;
    /** How much of the reward that this account earns is set to be compound */
    int32_t compounding_percent;
    /** Is the account weight split in an optimal way */
    bool is_optimal;

    WitnessAccountStatisticsRecord(std::string request_status_,
                                   std::string account_status_,
                                   int64_t account_weight_,
                                   int64_t account_weight_at_creation_,
                                   int64_t account_parts_,
                                   int64_t account_amount_locked_,
                                   int64_t account_amount_locked_at_creation_,
                                   int64_t network_tip_total_weight_,
                                   int64_t network_total_weight_at_creation_,
                                   int64_t account_initial_lock_period_in_blocks_,
                                   int64_t account_remaining_lock_period_in_blocks_,
                                   int64_t account_expected_witness_period_in_blocks_,
                                   int64_t account_estimated_witness_period_in_blocks_,
                                   int64_t account_initial_lock_creation_block_height_,
                                   int32_t compounding_percent_,
                                   bool is_optimal_)
    : request_status(std::move(request_status_))
    , account_status(std::move(account_status_))
    , account_weight(std::move(account_weight_))
    , account_weight_at_creation(std::move(account_weight_at_creation_))
    , account_parts(std::move(account_parts_))
    , account_amount_locked(std::move(account_amount_locked_))
    , account_amount_locked_at_creation(std::move(account_amount_locked_at_creation_))
    , network_tip_total_weight(std::move(network_tip_total_weight_))
    , network_total_weight_at_creation(std::move(network_total_weight_at_creation_))
    , account_initial_lock_period_in_blocks(std::move(account_initial_lock_period_in_blocks_))
    , account_remaining_lock_period_in_blocks(std::move(account_remaining_lock_period_in_blocks_))
    , account_expected_witness_period_in_blocks(std::move(account_expected_witness_period_in_blocks_))
    , account_estimated_witness_period_in_blocks(std::move(account_estimated_witness_period_in_blocks_))
    , account_initial_lock_creation_block_height(std::move(account_initial_lock_creation_block_height_))
    , compounding_percent(std::move(compounding_percent_))
    , is_optimal(std::move(is_optimal_))
    {}
};
