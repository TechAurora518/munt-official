// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <utility>

/** monitoring stats */
struct MonitorRecord final {
    int32_t partialHeight;
    int32_t partialOffset;
    int32_t prunedHeight;
    int32_t processedSPVHeight;
    int32_t probableHeight;

    MonitorRecord(int32_t partialHeight_,
                  int32_t partialOffset_,
                  int32_t prunedHeight_,
                  int32_t processedSPVHeight_,
                  int32_t probableHeight_)
    : partialHeight(std::move(partialHeight_))
    , partialOffset(std::move(partialOffset_))
    , prunedHeight(std::move(prunedHeight_))
    , processedSPVHeight(std::move(processedSPVHeight_))
    , probableHeight(std::move(probableHeight_))
    {}
};
