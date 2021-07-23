// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import <Foundation/Foundation.h>
@protocol DBIGenerationListener;


/** C++ interface to control generation of blocks (proof of work) */
@interface DBIGenerationController : NSObject

/** Register listener to be notified of generation related events */
+ (void)setListener:(nullable id<DBIGenerationListener>)generationListener;

/**
 * Activate block generation (proof of work)
 * Number of threads should not exceed physical threads, memory limit is a string specifier in the form of #B/#K/#M/#G (e.g. 102400B, 10240K, 1024M, 1G)
 */
+ (BOOL)startGeneration:(int32_t)numThreads
        numArenaThreads:(int32_t)numArenaThreads
            memoryLimit:(nonnull NSString *)memoryLimit;

/** Stop any active block generation (proof of work) */
+ (BOOL)stopGeneration;

/**
 * Get the address of the account that is used for generation by default. Empty on failiure
 * Note that this isn't necessarily the actual generation address as there might be an override
 * See: getGenerationOverrideAddress
 */
+ (nonnull NSString *)getGenerationAddress;

/**
 * Get the 'override' address for generation, if one has been set
 * The override address, when present it used for all block generation in place of the default account address
 */
+ (nonnull NSString *)getGenerationOverrideAddress;

/** Set an override address to use for block generation in place of the default */
+ (BOOL)setGenerationOverrideAddress:(nonnull NSString *)overrideAddress;

+ (int64_t)getAvailableCores;

+ (int64_t)getMinimumMemory;

+ (int64_t)getMaximumMemory;

@end
