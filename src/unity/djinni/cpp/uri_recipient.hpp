// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <string>
#include <utility>

struct UriRecipient final {
    bool valid;
    std::string address;
    std::string label;
    std::string description;
    int64_t amount;

    UriRecipient(bool valid_,
                 std::string address_,
                 std::string label_,
                 std::string description_,
                 int64_t amount_)
    : valid(std::move(valid_))
    , address(std::move(address_))
    , label(std::move(label_))
    , description(std::move(description_))
    , amount(std::move(amount_))
    {}
};
