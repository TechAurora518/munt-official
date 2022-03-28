// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NativeIWalletListener.hpp"  // my header
#include "Marshal.hpp"
#include "NativeBalanceRecord.hpp"
#include "NativeMutationRecord.hpp"
#include "NativeTransactionRecord.hpp"

namespace djinni_generated {

NativeIWalletListener::NativeIWalletListener() : ::djinni::JniInterface<::IWalletListener, NativeIWalletListener>() {}

NativeIWalletListener::~NativeIWalletListener() = default;

NativeIWalletListener::JavaProxy::JavaProxy(JniType j) : Handle(::djinni::jniGetThreadEnv(), j) { }

NativeIWalletListener::JavaProxy::~JavaProxy() = default;

void NativeIWalletListener::JavaProxy::notifyBalanceChange(const ::BalanceRecord & c_new_balance) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::NativeIWalletListener>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_notifyBalanceChange,
                           ::djinni::get(::djinni_generated::NativeBalanceRecord::fromCpp(jniEnv, c_new_balance)));
    ::djinni::jniExceptionCheck(jniEnv);
}
void NativeIWalletListener::JavaProxy::notifyNewMutation(const ::MutationRecord & c_mutation, bool c_self_committed) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::NativeIWalletListener>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_notifyNewMutation,
                           ::djinni::get(::djinni_generated::NativeMutationRecord::fromCpp(jniEnv, c_mutation)),
                           ::djinni::get(::djinni::Bool::fromCpp(jniEnv, c_self_committed)));
    ::djinni::jniExceptionCheck(jniEnv);
}
void NativeIWalletListener::JavaProxy::notifyUpdatedTransaction(const ::TransactionRecord & c_transaction) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::NativeIWalletListener>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_notifyUpdatedTransaction,
                           ::djinni::get(::djinni_generated::NativeTransactionRecord::fromCpp(jniEnv, c_transaction)));
    ::djinni::jniExceptionCheck(jniEnv);
}
void NativeIWalletListener::JavaProxy::notifyWalletUnlocked() {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::NativeIWalletListener>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_notifyWalletUnlocked);
    ::djinni::jniExceptionCheck(jniEnv);
}
void NativeIWalletListener::JavaProxy::notifyWalletLocked() {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::NativeIWalletListener>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_notifyWalletLocked);
    ::djinni::jniExceptionCheck(jniEnv);
}
void NativeIWalletListener::JavaProxy::notifyCoreWantsUnlock(const std::string & c_reason) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::NativeIWalletListener>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_notifyCoreWantsUnlock,
                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c_reason)));
    ::djinni::jniExceptionCheck(jniEnv);
}
void NativeIWalletListener::JavaProxy::notifyCoreInfo(const std::string & c_type, const std::string & c_caption, const std::string & c_message) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::NativeIWalletListener>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_notifyCoreInfo,
                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c_type)),
                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c_caption)),
                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c_message)));
    ::djinni::jniExceptionCheck(jniEnv);
}

}  // namespace djinni_generated
