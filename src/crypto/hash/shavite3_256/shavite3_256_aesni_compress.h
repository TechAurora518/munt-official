// File originates from the supercop project
// Authors: Ryad Benadjila -- Orange Labs, Olivier Billet -- Orange Labs (June 2009(
// Modified (October 2010): Eli Biham and Orr Dunkelman (applying the SHAvite-3 tweak)
// Modified (August 2019) Conversion from asm to intrinsic instructions: David Wohlferd (https://stackoverflow.com/questions/57664080/access-thread-local-variable-in-inline-assembly/57684519#57684519)
//
// File contains further modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2019 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef SHAVITE_3_256_AESNI_COMPRESS_H
#define SHAVITE_3_256_AESNI_COMPRESS_H

#include <compat/arch.h>
// Only x86 family CPUs have AES-NI
#ifdef ARCH_CPU_X86_FAMILY

#ifndef __clang__
#pragma GCC push_options
#pragma GCC target("aes,ssse3")
#ifndef DEBUG
    #pragma GCC optimize ("O3")
#endif
#else
#pragma clang attribute push (__attribute__((target("aes,ssse3"))), apply_to=any(function))
#endif
#include <immintrin.h>

#define T8(x) ((x) & 0xff)

/// Encrypts the plaintext pt[] using the key message[], and counter[], to produce the ciphertext ct[]


#define SHAVITE_MIXING_256_OPT   \
    x11 = x15;                   \
    x10 = x14;                   \
    x9 =  x13;                   \
    x8 = x12;                    \
                                 \
    x6 = x11;                    \
    x6 = _mm_srli_si128(x6, 4);  \
    x8 = _mm_xor_si128(x8,  x6); \
    x6 = x8;                     \
    x6 = _mm_slli_si128(x6,  12);\
    x8 = _mm_xor_si128(x8, x6);  \
                                 \
    x7 = x8;                     \
    x7 =  _mm_srli_si128(x7,  4);\
    x9 = _mm_xor_si128(x9,  x7); \
    x7 = x9;                     \
    x7 = _mm_slli_si128(x7, 12); \
    x9 = _mm_xor_si128(x9, x7);  \
                                 \
    x6 = x9;                     \
    x6 =  _mm_srli_si128(x6, 4); \
    x10 = _mm_xor_si128(x10, x6);\
    x6 = x10;                    \
    x6 = _mm_slli_si128(x6,  12);\
    x10 = _mm_xor_si128(x10, x6);\
                                 \
    x7 = x10;                    \
    x7 = _mm_srli_si128(x7,  4); \
    x11 = _mm_xor_si128(x11, x7);\
    x7 = x11;                    \
    x7 = _mm_slli_si128(x7,  12);\
    x11 = _mm_xor_si128(x11, x7);

// encryption + Davies-Meyer transform
void shavite3_256_aesni_Compress256(const unsigned char* message_block, unsigned char* chaining_value, uint64_t counter)
{
    __attribute__ ((aligned (16))) static const unsigned int SHAVITE_REVERSE[4] = {0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0x03020100 };
    __attribute__ ((aligned (16))) static const unsigned int SHAVITE256_XOR2[4] = {0x0, 0xFFFFFFFF, 0x0, 0x0};
    __attribute__ ((aligned (16))) static const unsigned int SHAVITE256_XOR3[4] = {0x0, 0x0, 0xFFFFFFFF, 0x0};
    __attribute__ ((aligned (16))) static const unsigned int SHAVITE256_XOR4[4] = {0x0, 0x0, 0x0, 0xFFFFFFFF};
    __attribute__ ((aligned (16))) const unsigned int SHAVITE_CNTS[4] = {(unsigned int)(counter & 0xFFFFFFFFULL),(unsigned int)(counter>>32),0,0}; 

    __m128i x0;
    __m128i x1;
    __m128i x2;
    __m128i x3;
    __m128i x4;
    __m128i x6;
    __m128i x7;
    __m128i x8;
    __m128i x9;
    __m128i x10;
    __m128i x11;
    __m128i x12;
    __m128i x13;
    __m128i x14;
    __m128i x15;

    // (L,R) = (xmm0,xmm1)
    const __m128i ptxt1 = _mm_loadu_si128((const __m128i*)chaining_value);
    const __m128i ptxt2 = _mm_loadu_si128((const __m128i*)(chaining_value+16));

    x0 = ptxt1;
    x1 = ptxt2;

    x3 = _mm_loadu_si128((__m128i*)SHAVITE_CNTS);
    x4 = _mm_loadu_si128((__m128i*)SHAVITE256_XOR2);
    x2 = _mm_setzero_si128();

    // init key schedule
    x8 = _mm_loadu_si128((__m128i*)message_block);
    x9 = _mm_loadu_si128((__m128i*)(((unsigned int*)message_block)+4));
    x10 = _mm_loadu_si128((__m128i*)(((unsigned int*)message_block)+8));
    x11 = _mm_loadu_si128((__m128i*)(((unsigned int*)message_block)+12));

    // xmm8..xmm11 = rk[0..15]
    // start key schedule
    x12 = x8;
    x13 = x9;
    x14 = x10;
    x15 = x11;

    const __m128i xtemp = _mm_loadu_si128((__m128i*)SHAVITE_REVERSE);
    x12 = _mm_shuffle_epi8(x12, xtemp);
    x13 = _mm_shuffle_epi8(x13, xtemp);
    x14 = _mm_shuffle_epi8(x14, xtemp);
    x15 = _mm_shuffle_epi8(x15, xtemp);

    x12 = _mm_aesenc_si128(x12, x2);
    x13 = _mm_aesenc_si128(x13, x2);
    x14 = _mm_aesenc_si128(x14, x2);
    x15 = _mm_aesenc_si128(x15, x2);

    x12 = _mm_xor_si128(x12, x3);
    x12 = _mm_xor_si128(x12, x4);
    x4 =  _mm_loadu_si128((__m128i*)SHAVITE256_XOR3);
    x12 = _mm_xor_si128(x12, x11);
    x13 = _mm_xor_si128(x13, x12);
    x14 = _mm_xor_si128(x14, x13);
    x15 = _mm_xor_si128(x15, x14);
   
    // xmm12..xmm15 = rk[16..31]
    // F3 - first round 
    x6 = x8;
    x8 = _mm_xor_si128(x8, x1);
    x8 = _mm_aesenc_si128(x8, x9);
    x8 = _mm_aesenc_si128(x8, x10);
    x8 = _mm_aesenc_si128(x8, x2);
    x0 = _mm_xor_si128(x0, x8);
    x8 = x6;

    // F3 - second round
    x6 = x11;
    x11 = _mm_xor_si128(x11, x0);
    x11 = _mm_aesenc_si128(x11, x12);
    x11 = _mm_aesenc_si128(x11, x13);
    x11 = _mm_aesenc_si128(x11, x2);
    x1 = _mm_xor_si128(x1, x11);
    x11 = x6;

    // key schedule
    SHAVITE_MIXING_256_OPT

    // xmm8..xmm11 - rk[32..47]
    // F3 - third round
    x6 = x14;
    x14 = _mm_xor_si128(x14, x1);
    x14 = _mm_aesenc_si128(x14, x15);
    x14 = _mm_aesenc_si128(x14, x8);
    x14 = _mm_aesenc_si128(x14, x2);
    x0 = _mm_xor_si128(x0, x14);
    x14 = x6;

    // key schedule
    x3 = _mm_shuffle_epi32(x3, 135);

    x12 = x8;
    x13 = x9;
    x14 = x10;
    x15 = x11;
    x12 = _mm_shuffle_epi8(x12, xtemp);
    x13 = _mm_shuffle_epi8(x13, xtemp);
    x14 = _mm_shuffle_epi8(x14, xtemp);
    x15 = _mm_shuffle_epi8(x15, xtemp);
    x12 = _mm_aesenc_si128(x12, x2);
    x13 = _mm_aesenc_si128(x13, x2);
    x14 = _mm_aesenc_si128(x14, x2);
    x15 = _mm_aesenc_si128(x15, x2);

    x12 = _mm_xor_si128(x12, x11);
    x14 = _mm_xor_si128(x14, x3);
    x14 = _mm_xor_si128(x14, x4);
    x4 = _mm_loadu_si128((__m128i*)SHAVITE256_XOR4);
    x13 = _mm_xor_si128(x13, x12);
    x14 = _mm_xor_si128(x14, x13);
    x15 = _mm_xor_si128(x15, x14);

    // xmm12..xmm15 - rk[48..63]

    // F3 - fourth round
    x6 = x9;
    x9 = _mm_xor_si128(x9, x0);
    x9 = _mm_aesenc_si128(x9, x10);
    x9 = _mm_aesenc_si128(x9, x11);
    x9 = _mm_aesenc_si128(x9, x2);
    x1 = _mm_xor_si128(x1, x9);
    x9 = x6;

    // key schedule
    SHAVITE_MIXING_256_OPT
    // xmm8..xmm11 = rk[64..79]
    // F3  - fifth round
    x6 = x12;
    x12 = _mm_xor_si128(x12, x1);
    x12 = _mm_aesenc_si128(x12, x13);
    x12 = _mm_aesenc_si128(x12, x14);
    x12 = _mm_aesenc_si128(x12, x2);
    x0 = _mm_xor_si128(x0, x12);
    x12 = x6;

    // F3 - sixth round
    x6 = x15;
    x15 = _mm_xor_si128(x15, x0);
    x15 = _mm_aesenc_si128(x15, x8);
    x15 = _mm_aesenc_si128(x15, x9);
    x15 = _mm_aesenc_si128(x15, x2);
    x1 = _mm_xor_si128(x1, x15);
    x15 = x6;

    // key schedule
    x3 = _mm_shuffle_epi32(x3, 147);

    x12 = x8;
    x13 = x9;
    x14 = x10;
    x15 = x11;
    x12 = _mm_shuffle_epi8(x12, xtemp);
    x13 = _mm_shuffle_epi8(x13, xtemp);
    x14 = _mm_shuffle_epi8(x14, xtemp);
    x15 = _mm_shuffle_epi8(x15, xtemp);
    x12 = _mm_aesenc_si128(x12, x2);
    x13 = _mm_aesenc_si128(x13, x2);
    x14 = _mm_aesenc_si128(x14, x2);
    x15 = _mm_aesenc_si128(x15, x2);
    x12 = _mm_xor_si128(x12, x11);
    x13 = _mm_xor_si128(x13, x3);
    x13 = _mm_xor_si128(x13, x4);
    x13 = _mm_xor_si128(x13, x12);
    x14 = _mm_xor_si128(x14, x13);
    x15 = _mm_xor_si128(x15, x14);

    // xmm12..xmm15 = rk[80..95]
    // F3 - seventh round
    x6 = x10;
    x10 = _mm_xor_si128(x10, x1);
    x10 = _mm_aesenc_si128(x10, x11);
    x10 = _mm_aesenc_si128(x10, x12);
    x10 = _mm_aesenc_si128(x10, x2);
    x0 = _mm_xor_si128(x0, x10);
    x10 = x6;

    // key schedule
    SHAVITE_MIXING_256_OPT

    // xmm8..xmm11 = rk[96..111]
    // F3 - eigth round
    x6 = x13;
    x13 = _mm_xor_si128(x13, x0);
    x13 = _mm_aesenc_si128(x13, x14);
    x13 = _mm_aesenc_si128(x13, x15);
    x13 = _mm_aesenc_si128(x13, x2);
    x1 = _mm_xor_si128(x1, x13);
    x13 = x6;


    // key schedule
    x3 = _mm_shuffle_epi32(x3, 135);

    x12 = x8;
    x13 = x9;
    x14 = x10;
    x15 = x11;
    x12 = _mm_shuffle_epi8(x12, xtemp);
    x13 = _mm_shuffle_epi8(x13, xtemp);
    x14 = _mm_shuffle_epi8(x14, xtemp);
    x15 = _mm_shuffle_epi8(x15, xtemp);
    x12 = _mm_aesenc_si128(x12, x2);
    x13 = _mm_aesenc_si128(x13, x2);
    x14 = _mm_aesenc_si128(x14, x2);
    x15 = _mm_aesenc_si128(x15, x2);
    x12 = _mm_xor_si128(x12, x11);
    x15 = _mm_xor_si128(x15, x3);
    x15 = _mm_xor_si128(x15, x4);
    x13 = _mm_xor_si128(x13, x12);
    x14 = _mm_xor_si128(x14, x13);
    x15 = _mm_xor_si128(x15, x14);

    // xmm12..xmm15 = rk[112..127]
    // F3 - ninth round
    x6 = x8;
    x8 = _mm_xor_si128(x8, x1);
    x8 = _mm_aesenc_si128(x8, x9);
    x8 = _mm_aesenc_si128(x8, x10);
    x8 = _mm_aesenc_si128(x8, x2);
    x0 = _mm_xor_si128(x0, x8);
    x8 = x6;
    // F3 - tenth round
    x6 = x11;
    x11 = _mm_xor_si128(x11, x0);
    x11 = _mm_aesenc_si128(x11, x12);
    x11 = _mm_aesenc_si128(x11, x13);
    x11 = _mm_aesenc_si128(x11, x2);
    x1 = _mm_xor_si128(x1, x11);
    x11 = x6;

    // key schedule
    SHAVITE_MIXING_256_OPT

    // xmm8..xmm11 = rk[128..143]
    // F3 - eleventh round
    x6 = x14;
    x14 = _mm_xor_si128(x14, x1);
    x14 = _mm_aesenc_si128(x14, x15);
    x14 = _mm_aesenc_si128(x14, x8);
    x14 = _mm_aesenc_si128(x14, x2);
    x0 = _mm_xor_si128(x0, x14);
    x14 = x6;

    // F3 - twelfth round
    x6 = x9;
    x9 = _mm_xor_si128(x9, x0);
    x9 = _mm_aesenc_si128(x9, x10);
    x9 = _mm_aesenc_si128(x9, x11);
    x9 = _mm_aesenc_si128(x9, x2);
    x1 = _mm_xor_si128(x1, x9);
    x9 = x6;


    // feedforward
    x0 = _mm_xor_si128(x0, ptxt1);
    x1 = _mm_xor_si128(x1, ptxt2);
    _mm_storeu_si128((__m128i *)chaining_value, x0);
    _mm_storeu_si128((__m128i *)(chaining_value + 16), x1);

    return;
}

#ifdef __clang__
#pragma clang attribute pop
#else
#pragma GCC pop_options
#endif

#endif
#endif
