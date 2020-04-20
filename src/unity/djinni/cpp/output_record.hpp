// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <string>
#include <utility>

struct OutputRecord final {
    int64_t amount;
    std::string address;
    std::string label;
    std::string description;
    bool isMine;

    OutputRecord(int64_t amount_,
                 std::string address_,
                 std::string label_,
                 std::string description_,
                 bool isMine_)
    : amount(std::move(amount_))
    , address(std::move(address_))
    , label(std::move(label_))
    , description(std::move(description_))
    , isMine(std::move(isMine_))
    {}
};
