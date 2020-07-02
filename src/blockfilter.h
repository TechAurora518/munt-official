// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2019 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef BLOCKFILTER_H
#define BLOCKFILTER_H

#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

#include <primitives/block.h>
#include <serialize.h>
#include <uint256.h>
#include <undo.h>
#include "chain.h"
#include <crypto/bytevectorhash.h>

/**
 * This implements a Golomb-coded set as defined in BIP 158. It is a
 * compact, probabilistic data structure for testing set membership.
 */
class GCSFilter
{
public:
    typedef std::vector<unsigned char> Element;
    typedef std::unordered_set<Element, ByteVectorHash> ElementSet;

    struct Params
    {
        uint64_t m_siphash_k0;
        uint64_t m_siphash_k1;
        uint8_t m_P;  //!< Golomb-Rice coding parameter
        uint32_t m_M;  //!< Inverse false positive rate

        Params(uint64_t siphash_k0 = 0, uint64_t siphash_k1 = 0, uint8_t P = 0, uint32_t M = 1)
            : m_siphash_k0(siphash_k0), m_siphash_k1(siphash_k1), m_P(P), m_M(M)
        {}
    };

private:
    Params m_params;
    uint32_t m_N;  //!< Number of elements in the filter
    uint64_t m_F;  //!< Range of element hashes, F = N * M
    std::vector<unsigned char> m_encoded;

    /** Hash a data element to an integer in the range [0, N * M). */
    uint64_t HashToRange(const Element& element) const;

    std::vector<uint64_t> BuildHashedSet(const ElementSet& elements) const;

    /** Helper method used to implement Match and MatchAny */
    bool MatchInternal(const uint64_t* sorted_element_hashes, size_t size) const;

public:

    /** Constructs an empty filter. */
    explicit GCSFilter(const Params& params = Params());

    /** Reconstructs an already-created filter from an encoding. */
    GCSFilter(const Params& params, std::vector<unsigned char> encoded_filter);

    /** Builds a new filter from the params and set of elements. */
    GCSFilter(const Params& params, const ElementSet& elements);

    uint32_t GetN() const { return m_N; }
    const Params& GetParams() const { return m_params; }
    const std::vector<unsigned char>& GetEncoded() const { return m_encoded; }

    /**
     * Checks if the element may be in the set. False positives are possible
     * with probability 1/M.
     */
    bool Match(const Element& element) const;

    /**
     * Checks if any of the given elements may be in the set. False positives
     * are possible with probability 1/M per element checked. This is more
     * efficient that checking Match on multiple elements separately.
     */
    bool MatchAny(const ElementSet& elements) const;
    unsigned int NumElements() const;
};

constexpr uint8_t BASIC_FILTER_P = 19;
constexpr uint32_t BASIC_FILTER_M = 784931;

enum BlockFilterType : uint8_t
{
    BASIC = 0,
};

/** Get the human-readable name for a filter type. Returns empty string for unknown types. */
const std::string& BlockFilterTypeName(BlockFilterType filter_type);

/** Find a filter type by its human-readable name. */
bool BlockFilterTypeByName(const std::string& name, BlockFilterType& filter_type);

/** Get a list of known filter types. */
const std::vector<BlockFilterType>& AllBlockFilterTypes();

/** Get a comma-separated list of known filter type names. */
std::string ListBlockFilterTypes();

/**
 * Complete block filter struct as defined in BIP 157. Serialization matches
 * payload of "cfilter" messages.
 */
class BlockFilter
{
protected:
    BlockFilterType m_filter_type;
    uint256 m_block_hash;
    GCSFilter m_filter;

    virtual bool BuildParams(GCSFilter::Params& params) const;

public:

    BlockFilter() = default;

    //! Reconstruct a BlockFilter from parts.
    BlockFilter(BlockFilterType filter_type, const uint256& block_hash,
                std::vector<unsigned char> filter);

    //! Construct a new BlockFilter of the specified type from a block.
    BlockFilter(BlockFilterType filter_type, const CBlock& block, const CBlockUndo& block_undo);

    BlockFilterType GetFilterType() const { return m_filter_type; }
    const uint256& GetBlockHash() const { return m_block_hash; }
    const GCSFilter& GetFilter() const { return m_filter; }

    const std::vector<unsigned char>& GetEncodedFilter() const
    {
        return m_filter.GetEncoded();
    }

    //! Compute the filter hash.
    uint256 GetHash() const;

    //! Compute the filter header given the previous one.
    uint256 ComputeHeader(const uint256& prev_header) const;

    template <typename Stream>
    void Serialize(Stream& s) const {
        s << m_block_hash
          << static_cast<uint8_t>(m_filter_type)
          << m_filter.GetEncoded();
    }

    template <typename Stream>
    void Unserialize(Stream& s) {
        std::vector<unsigned char> encoded_filter;
        uint8_t filter_type;

        s >> m_block_hash
          >> filter_type
          >> encoded_filter;

        m_filter_type = static_cast<BlockFilterType>(filter_type);

        GCSFilter::Params params;
        if (!BuildParams(params)) {
            throw std::ios_base::failure("unknown filter_type");
        }
        m_filter = GCSFilter(params, std::move(encoded_filter));
    }
};


//! Special sub class used only for filtercp used by SPV sync, not for network messages or anything consensus related.
class RangedCPBlockFilter : public BlockFilter
{
public:
    //! Construct a new RangedCPBlockFilter of the specified type from a range.
    //! Filter contains startRange->endRange - inclusive of start range, exclusive of end range
    RangedCPBlockFilter(const CBlockIndex* startRange, const CBlockIndex* endRange);

    //! Reconstruct from parts.
    RangedCPBlockFilter(std::vector<unsigned char> filter);
private:
    virtual bool BuildParams(GCSFilter::Params& params) const;
};

void getBlockFilterBirthAndRanges(uint64_t nHardBirthDate, uint64_t& nSoftBirthDate, const GCSFilter::ElementSet& walletAddresses, std::vector<std::tuple<uint64_t, uint64_t>>& blockFilterRanges);

#endif
