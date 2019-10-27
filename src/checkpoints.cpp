// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"

#include "chain.h"
#include "chainparams.h"
#include "validation/validation.h"
#include "uint256.h"

#include <boost/foreach.hpp>
#include <stdint.h>
#include <boost/range/rbegin.hpp>

namespace Checkpoints {

    CBlockIndex* GetLastCheckpoint(const CCheckpointData& data)
    {
        const MapCheckpoints& checkpoints = data.mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            BlockMap::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    unsigned int LastCheckPointHeight()
    {
        if (Params().Checkpoints().mapCheckpoints.size() > 0)
        {
            auto lastCheckpoint = Params().Checkpoints().mapCheckpoints.rbegin();
            return lastCheckpoint->first;
        }
        else
        {
            return 0;
        }
    }
} // namespace Checkpoints
