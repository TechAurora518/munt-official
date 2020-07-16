// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#ifdef DJINNI_NODEJS
#include "NJSIGenerationListener.hpp"
#else
class IGenerationListener;
#endif

/** C++ interface to control generation of blocks (proof of work) */
class IGenerationController {
public:
    virtual ~IGenerationController() {}

    /** Register listener to be notified of generation related events */
    static void setListener(const std::shared_ptr<IGenerationListener> & generationListener);

    /**
     * Activate block generation (proof of work)
     * Number of threads should not exceed physical threads, memory limit is a string specifier in the form of #B/#K/#M/#G (e.g. 102400B, 10240K, 1024M, 1G)
     */
    static bool startGeneration(int32_t numThreads, const std::string & memoryLimit);

    /** Stop any active block generation (proof of work) */
    static bool stopGeneration();
};
