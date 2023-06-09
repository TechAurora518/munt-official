// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NativeIWitnessController.hpp"  // my header
#include "Marshal.hpp"
#include "NativeResultRecord.hpp"
#include "NativeWitnessAccountStatisticsRecord.hpp"
#include "NativeWitnessEstimateInfoRecord.hpp"
#include "NativeWitnessFundingResultRecord.hpp"

namespace djinni_generated {

NativeIWitnessController::NativeIWitnessController() : ::djinni::JniInterface<::IWitnessController, NativeIWitnessController>("unity_wallet/jniunifiedbackend/IWitnessController$CppProxy") {}

NativeIWitnessController::~NativeIWitnessController() = default;


CJNIEXPORT void JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::IWitnessController>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_getNetworkLimits(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getNetworkLimits();
        return ::djinni::release(::djinni::Map<::djinni::String, ::djinni::String>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_getEstimatedWeight(JNIEnv* jniEnv, jobject /*this*/, jlong j_amountToLock, jlong j_lockPeriodInBlocks)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getEstimatedWeight(::djinni::I64::toCpp(jniEnv, j_amountToLock),
                                                          ::djinni::I64::toCpp(jniEnv, j_lockPeriodInBlocks));
        return ::djinni::release(::djinni_generated::NativeWitnessEstimateInfoRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_fundWitnessAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_fundingAccountUUID, jstring j_witnessAccountUUID, jlong j_fundingAmount, jlong j_requestedLockPeriodInBlocks)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::fundWitnessAccount(::djinni::String::toCpp(jniEnv, j_fundingAccountUUID),
                                                          ::djinni::String::toCpp(jniEnv, j_witnessAccountUUID),
                                                          ::djinni::I64::toCpp(jniEnv, j_fundingAmount),
                                                          ::djinni::I64::toCpp(jniEnv, j_requestedLockPeriodInBlocks));
        return ::djinni::release(::djinni_generated::NativeWitnessFundingResultRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_renewWitnessAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_fundingAccountUUID, jstring j_witnessAccountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::renewWitnessAccount(::djinni::String::toCpp(jniEnv, j_fundingAccountUUID),
                                                           ::djinni::String::toCpp(jniEnv, j_witnessAccountUUID));
        return ::djinni::release(::djinni_generated::NativeWitnessFundingResultRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_getAccountWitnessStatistics(JNIEnv* jniEnv, jobject /*this*/, jstring j_witnessAccountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getAccountWitnessStatistics(::djinni::String::toCpp(jniEnv, j_witnessAccountUUID));
        return ::djinni::release(::djinni_generated::NativeWitnessAccountStatisticsRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_setAccountCompounding(JNIEnv* jniEnv, jobject /*this*/, jstring j_witnessAccountUUID, jint j_percentToCompount)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        ::IWitnessController::setAccountCompounding(::djinni::String::toCpp(jniEnv, j_witnessAccountUUID),
                                                    ::djinni::I32::toCpp(jniEnv, j_percentToCompount));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jint JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_isAccountCompounding(JNIEnv* jniEnv, jobject /*this*/, jstring j_witnessAccountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::isAccountCompounding(::djinni::String::toCpp(jniEnv, j_witnessAccountUUID));
        return ::djinni::release(::djinni::I32::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_getWitnessAddress(JNIEnv* jniEnv, jobject /*this*/, jstring j_witnessAccountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getWitnessAddress(::djinni::String::toCpp(jniEnv, j_witnessAccountUUID));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_getOptimalWitnessDistribution(JNIEnv* jniEnv, jobject /*this*/, jlong j_amount, jlong j_durationInBlocks, jlong j_totalNetworkWeight)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getOptimalWitnessDistribution(::djinni::I64::toCpp(jniEnv, j_amount),
                                                                     ::djinni::I64::toCpp(jniEnv, j_durationInBlocks),
                                                                     ::djinni::I64::toCpp(jniEnv, j_totalNetworkWeight));
        return ::djinni::release(::djinni::List<::djinni::I64>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_getOptimalWitnessDistributionForAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_witnessAccountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getOptimalWitnessDistributionForAccount(::djinni::String::toCpp(jniEnv, j_witnessAccountUUID));
        return ::djinni::release(::djinni::List<::djinni::I64>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IWitnessController_00024CppProxy_optimiseWitnessAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_witnessAccountUUID, jstring j_fundingAccountUUID, jobject j_optimalDistribution)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::optimiseWitnessAccount(::djinni::String::toCpp(jniEnv, j_witnessAccountUUID),
                                                              ::djinni::String::toCpp(jniEnv, j_fundingAccountUUID),
                                                              ::djinni::List<::djinni::I64>::toCpp(jniEnv, j_optimalDistribution));
        return ::djinni::release(::djinni_generated::NativeResultRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
