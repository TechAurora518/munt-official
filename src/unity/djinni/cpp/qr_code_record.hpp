// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

/**
 * Copyright (c) 2018-2020 The Gulden developers
 * Authored by: Malcolm MacLeod (mmacleod@gmx.com)
 * Distributed under the Libre Chain License, see the accompanying
 * file COPYING
 */
struct QrCodeRecord final {
    int32_t width;
    std::vector<uint8_t> pixel_data;

    QrCodeRecord(int32_t width_,
                 std::vector<uint8_t> pixel_data_)
    : width(std::move(width_))
    , pixel_data(std::move(pixel_data_))
    {}
};
