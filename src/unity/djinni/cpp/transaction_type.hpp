// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <functional>

enum class TransactionType : int {
    SEND,
    RECEIVE,
};

namespace std {

template <>
struct hash<::TransactionType> {
    size_t operator()(::TransactionType type) const {
        return std::hash<int>()(static_cast<int>(type));
    }
};

}  // namespace std
