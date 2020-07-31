// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NativeIWitnessController.hpp"  // my header
#include "Marshal.hpp"
#include "NativeWitnessEstimateInfoRecord.hpp"
#include "NativeWitnessFundingResultRecord.hpp"

namespace djinni_generated {

NativeIWitnessController::NativeIWitnessController() : ::djinni::JniInterface<::IWitnessController, NativeIWitnessController>("com/novo/jniunifiedbackend/IWitnessController$CppProxy") {}

NativeIWitnessController::~NativeIWitnessController() = default;


CJNIEXPORT void JNICALL Java_com_novo_jniunifiedbackend_IWitnessController_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::IWitnessController>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_com_novo_jniunifiedbackend_IWitnessController_00024CppProxy_getNetworkLimits(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getNetworkLimits();
        return ::djinni::release(::djinni::Map<::djinni::String, ::djinni::String>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_com_novo_jniunifiedbackend_IWitnessController_00024CppProxy_getEstimatedWeight(JNIEnv* jniEnv, jobject /*this*/, jlong j_amountToLock, jlong j_lockPeriodInDays)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWitnessController::getEstimatedWeight(::djinni::I64::toCpp(jniEnv, j_amountToLock),
                                                          ::djinni::I64::toCpp(jniEnv, j_lockPeriodInDays));
        return ::djinni::release(::djinni_generated::NativeWitnessEstimateInfoRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_com_novo_jniunifiedbackend_IWitnessController_00024CppProxy_fundWitnessAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_fundingAccountUUID, jstring j_witnessAccountUUID, jlong j_fundingAmount, jlong j_requestedLockPeriodInBlocks)
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

}  // namespace djinni_generated
