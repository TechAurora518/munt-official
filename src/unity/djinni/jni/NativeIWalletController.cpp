// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NativeIWalletController.hpp"  // my header
#include "Marshal.hpp"
#include "NativeBalanceRecord.hpp"
#include "NativeIWalletListener.hpp"

namespace djinni_generated {

NativeIWalletController::NativeIWalletController() : ::djinni::JniInterface<::IWalletController, NativeIWalletController>("com/gulden/jniunifiedbackend/IWalletController$CppProxy") {}

NativeIWalletController::~NativeIWalletController() = default;


CJNIEXPORT void JNICALL Java_com_gulden_jniunifiedbackend_IWalletController_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::IWalletController>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_com_gulden_jniunifiedbackend_IWalletController_00024CppProxy_setListener(JNIEnv* jniEnv, jobject /*this*/, jobject j_networklistener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        ::IWalletController::setListener(::djinni_generated::NativeIWalletListener::toCpp(jniEnv, j_networklistener));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jboolean JNICALL Java_com_gulden_jniunifiedbackend_IWalletController_00024CppProxy_HaveUnconfirmedFunds(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWalletController::HaveUnconfirmedFunds();
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jlong JNICALL Java_com_gulden_jniunifiedbackend_IWalletController_00024CppProxy_GetBalanceSimple(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWalletController::GetBalanceSimple();
        return ::djinni::release(::djinni::I64::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_com_gulden_jniunifiedbackend_IWalletController_00024CppProxy_GetBalance(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWalletController::GetBalance();
        return ::djinni::release(::djinni_generated::NativeBalanceRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_com_gulden_jniunifiedbackend_IWalletController_00024CppProxy_AbandonTransaction(JNIEnv* jniEnv, jobject /*this*/, jstring j_txHash)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IWalletController::AbandonTransaction(::djinni::String::toCpp(jniEnv, j_txHash));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
