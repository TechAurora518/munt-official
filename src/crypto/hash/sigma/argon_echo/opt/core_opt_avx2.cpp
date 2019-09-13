// Copyright (c) 2019 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

// This file is a thin wrapper around the actual 'argon2_echo_opt' implementation, along with various other similarly named files.
// The build system compiles each file with slightly different optimisation flags so that we have optimised implementations for a wide spread of processors.

#include <compat/arch.h>
#if defined(ARCH_CPU_X86_FAMILY)

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../blake2/blake2.h"
#include "../blake2/blamka-round-opt_avx2.h"
#include "../../../echo256/echo256_opt.h"
#include "../argon_echo.h"
#include "../core.h"

#define ARGON2_BLOCK_WORD_SIZE __m256i
#define ARGON2_BLOCK_WORD_COUNT ARGON2_HWORDS_IN_BLOCK
#define next_addresses next_addresses_avx2
#define fill_segment fill_segment_avx2
#define fill_block fill_block_avx2
#define initialize initialize_avx2
#define fill_memory_blocks fill_memory_blocks_avx2
#define argon2_echo_ctx argon2_echo_ctx_avx2
#define Compress argon2_echo_compress_avx2
#define init_block_value argon2_echo_init_block_value_avx2
#define copy_block argon2_echo_copy_block_value_avx2
#define xor_block argon2_echo_xor_block_value_avx2
#define finalize argon2_echo_finalize_avx2
#define index_alpha argon2_echo_index_alpha_avx2
#define initial_hash argon2_echo_initial_hash_avx2
#define fill_first_blocks argon2_echo_fill_first_blocks_avx2
#define ECHO_HASH_256(DATA, DATABYTELEN, HASH)                                     \
{                                                                                  \
    echo256_opt_hashState ctx_echo;                                                \
    echo256_opt_avx2_Init(&ctx_echo);                                              \
    echo256_opt_avx2_Update(&ctx_echo, (const unsigned char*)(DATA), DATABYTELEN); \
    echo256_opt_avx2_Final(&ctx_echo, HASH);                                       \
}

static void fill_block_avx2(__m256i* state, const argon2_echo_block* ref_block, argon2_echo_block* next_block, int with_xor)
{
    __m256i block_XY[ARGON2_HWORDS_IN_BLOCK];
    unsigned int i;

    if (with_xor)
    {
        for (i = 0; i < ARGON2_HWORDS_IN_BLOCK; i++)
        {
            state[i] = _mm256_xor_si256(state[i], _mm256_loadu_si256((const __m256i *)ref_block->v + i));
            block_XY[i] = _mm256_xor_si256(state[i], _mm256_loadu_si256((const __m256i *)next_block->v + i));
        }
    }
    else
    {
        for (i = 0; i < ARGON2_HWORDS_IN_BLOCK; i++)
        {
            block_XY[i] = state[i] = _mm256_xor_si256(state[i], _mm256_loadu_si256((const __m256i *)ref_block->v + i));
        }
    }

    for (i = 0; i < 4; ++i)
    {
        BLAKE2_ROUND_AVX2_1(state[8 * i + 0], state[8 * i + 4], state[8 * i + 1], state[8 * i + 5], state[8 * i + 2], state[8 * i + 6], state[8 * i + 3], state[8 * i + 7]);
    }

    for (i = 0; i < 4; ++i)
    {
        BLAKE2_ROUND_AVX2_2(state[ 0 + i], state[ 4 + i], state[ 8 + i], state[12 + i], state[16 + i], state[20 + i], state[24 + i], state[28 + i]);
    }

    for (i = 0; i < ARGON2_HWORDS_IN_BLOCK; i++)
    {
        state[i] = _mm256_xor_si256(state[i], block_XY[i]);
        _mm256_storeu_si256((__m256i *)next_block->v + i, state[i]);
    }
}

#define ARGON2_CORE_OPT_IMPL
#include "../core.cpp"
#include "../argon2.cpp"
#endif
