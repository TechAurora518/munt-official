// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NativeIAccountsController.hpp"  // my header
#include "Marshal.hpp"
#include "NativeAccountLinkRecord.hpp"
#include "NativeAccountRecord.hpp"
#include "NativeBalanceRecord.hpp"
#include "NativeIAccountsListener.hpp"
#include "NativeMutationRecord.hpp"
#include "NativeTransactionRecord.hpp"

namespace djinni_generated {

NativeIAccountsController::NativeIAccountsController() : ::djinni::JniInterface<::IAccountsController, NativeIAccountsController>("unity_wallet/jniunifiedbackend/IAccountsController$CppProxy") {}

NativeIAccountsController::~NativeIAccountsController() = default;


CJNIEXPORT void JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::IAccountsController>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_setListener(JNIEnv* jniEnv, jobject /*this*/, jobject j_accountslistener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        ::IAccountsController::setListener(::djinni_generated::NativeIAccountsListener::toCpp(jniEnv, j_accountslistener));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_listAccounts(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::listAccounts();
        return ::djinni::release(::djinni::List<::djinni_generated::NativeAccountRecord>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_setActiveAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::setActiveAccount(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getActiveAccount(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getActiveAccount();
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_createAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountName, jstring j_accountType)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::createAccount(::djinni::String::toCpp(jniEnv, j_accountName),
                                                      ::djinni::String::toCpp(jniEnv, j_accountType));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getAccountName(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getAccountName(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_renameAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID, jstring j_newAccountName)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::renameAccount(::djinni::String::toCpp(jniEnv, j_accountUUID),
                                                      ::djinni::String::toCpp(jniEnv, j_newAccountName));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_deleteAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::deleteAccount(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_purgeAccount(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::purgeAccount(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getAccountLinkURI(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getAccountLinkURI(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getWitnessKeyURI(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getWitnessKeyURI(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_createAccountFromWitnessKeyURI(JNIEnv* jniEnv, jobject /*this*/, jstring j_witnessKeyURI, jstring j_newAccountName)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::createAccountFromWitnessKeyURI(::djinni::String::toCpp(jniEnv, j_witnessKeyURI),
                                                                       ::djinni::String::toCpp(jniEnv, j_newAccountName));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getReceiveAddress(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getReceiveAddress(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getTransactionHistory(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getTransactionHistory(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::List<::djinni_generated::NativeTransactionRecord>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getMutationHistory(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getMutationHistory(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::List<::djinni_generated::NativeMutationRecord>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getActiveAccountBalance(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getActiveAccountBalance();
        return ::djinni::release(::djinni_generated::NativeBalanceRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getAccountBalance(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getAccountBalance(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni_generated::NativeBalanceRecord::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_getAllAccountBalances(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::getAllAccountBalances();
        return ::djinni::release(::djinni::Map<::djinni::String, ::djinni_generated::NativeBalanceRecord>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_addAccountLink(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID, jstring j_serviceName, jstring j_data)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::addAccountLink(::djinni::String::toCpp(jniEnv, j_accountUUID),
                                                       ::djinni::String::toCpp(jniEnv, j_serviceName),
                                                       ::djinni::String::toCpp(jniEnv, j_data));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_removeAccountLink(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID, jstring j_serviceName)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::removeAccountLink(::djinni::String::toCpp(jniEnv, j_accountUUID),
                                                          ::djinni::String::toCpp(jniEnv, j_serviceName));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_unity_1wallet_jniunifiedbackend_IAccountsController_00024CppProxy_listAccountLinks(JNIEnv* jniEnv, jobject /*this*/, jstring j_accountUUID)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::IAccountsController::listAccountLinks(::djinni::String::toCpp(jniEnv, j_accountUUID));
        return ::djinni::release(::djinni::List<::djinni_generated::NativeAccountLinkRecord>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
