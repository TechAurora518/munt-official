// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBIGenerationController+Private.h"
#import "DBIGenerationController.h"
#import "DBIGenerationListener+Private.h"
#import "DJICppWrapperCache+Private.h"
#import "DJIError.h"
#import "DJIMarshal+Private.h"
#include <exception>
#include <stdexcept>
#include <utility>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

@interface DBIGenerationController ()

- (id)initWithCpp:(const std::shared_ptr<::IGenerationController>&)cppRef;

@end

@implementation DBIGenerationController {
    ::djinni::CppProxyCache::Handle<std::shared_ptr<::IGenerationController>> _cppRefHandle;
}

- (id)initWithCpp:(const std::shared_ptr<::IGenerationController>&)cppRef
{
    if (self = [super init]) {
        _cppRefHandle.assign(cppRef);
    }
    return self;
}

+ (void)setListener:(nullable id<DBIGenerationListener>)generationListener {
    try {
        ::IGenerationController::setListener(::djinni_generated::IGenerationListener::toCpp(generationListener));
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (BOOL)startGeneration:(int32_t)numThreads
        numArenaThreads:(int32_t)numArenaThreads
            memoryLimit:(nonnull NSString *)memoryLimit {
    try {
        auto objcpp_result_ = ::IGenerationController::startGeneration(::djinni::I32::toCpp(numThreads),
                                                                       ::djinni::I32::toCpp(numArenaThreads),
                                                                       ::djinni::String::toCpp(memoryLimit));
        return ::djinni::Bool::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (BOOL)stopGeneration {
    try {
        auto objcpp_result_ = ::IGenerationController::stopGeneration();
        return ::djinni::Bool::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)getGenerationAddress {
    try {
        auto objcpp_result_ = ::IGenerationController::getGenerationAddress();
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)getGenerationOverrideAddress {
    try {
        auto objcpp_result_ = ::IGenerationController::getGenerationOverrideAddress();
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (BOOL)setGenerationOverrideAddress:(nonnull NSString *)overrideAddress {
    try {
        auto objcpp_result_ = ::IGenerationController::setGenerationOverrideAddress(::djinni::String::toCpp(overrideAddress));
        return ::djinni::Bool::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (int64_t)getAvailableCores {
    try {
        auto objcpp_result_ = ::IGenerationController::getAvailableCores();
        return ::djinni::I64::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (int64_t)getMinimumMemory {
    try {
        auto objcpp_result_ = ::IGenerationController::getMinimumMemory();
        return ::djinni::I64::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (int64_t)getMaximumMemory {
    try {
        auto objcpp_result_ = ::IGenerationController::getMaximumMemory();
        return ::djinni::I64::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

namespace djinni_generated {

auto IGenerationController::toCpp(ObjcType objc) -> CppType
{
    if (!objc) {
        return nullptr;
    }
    return objc->_cppRefHandle.get();
}

auto IGenerationController::fromCppOpt(const CppOptType& cpp) -> ObjcType
{
    if (!cpp) {
        return nil;
    }
    return ::djinni::get_cpp_proxy<DBIGenerationController>(cpp);
}

}  // namespace djinni_generated

@end
