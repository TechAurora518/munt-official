// Copyright (c) 2019 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

// This file is a thin wrapper around the actual 'shavite3_256_aesni_opt' implementation, along with various other similarly named files.
// The build system compiles each file with slightly different optimisation flags so that we have optimised implementations for a wide spread of processors.

#include <compat/arch.h>

#if defined(ARCH_CPU_X86_FAMILY)
    #define shavite3_256_opt_Init        shavite3_256_opt_avx2_aes_Init
    #define shavite3_256_opt_Update      shavite3_256_opt_avx2_aes_Update
    #define shavite3_256_opt_Final       shavite3_256_opt_avx2_aes_Final
    #define shavite3_256_opt_UpdateFinal shavite3_256_opt_avx2_aes_UpdateFinal
    #define shavite3_256_opt_Compress256 shavite3_256_opt_avx2_aes_Compress256

    #define USE_HARDWARE_AES
    #define SHAVITE3_256_OPT_IMPL
    #include "../shavite3_256_opt.cpp"
#endif
