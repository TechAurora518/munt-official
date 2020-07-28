// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include "djinni_support.hpp"
#include "i_library_listener.hpp"

namespace djinni_generated {

class NativeILibraryListener final : ::djinni::JniInterface<::ILibraryListener, NativeILibraryListener> {
public:
    using CppType = std::shared_ptr<::ILibraryListener>;
    using CppOptType = std::shared_ptr<::ILibraryListener>;
    using JniType = jobject;

    using Boxed = NativeILibraryListener;

    ~NativeILibraryListener();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<NativeILibraryListener>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<NativeILibraryListener>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    NativeILibraryListener();
    friend ::djinni::JniClass<NativeILibraryListener>;
    friend ::djinni::JniInterface<::ILibraryListener, NativeILibraryListener>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ILibraryListener
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void notifyUnifiedProgress(float progress) override;
        void notifyBalanceChange(const ::BalanceRecord & new_balance) override;
        void notifyNewMutation(const ::MutationRecord & mutation, bool self_committed) override;
        void notifyUpdatedTransaction(const ::TransactionRecord & transaction) override;
        void notifyInitWithExistingWallet() override;
        void notifyInitWithoutExistingWallet() override;
        void notifyShutdown() override;
        void notifyCoreReady() override;
        void logPrint(const std::string & str) override;

    private:
        friend ::djinni::JniInterface<::ILibraryListener, ::djinni_generated::NativeILibraryListener>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("com/novo/jniunifiedbackend/ILibraryListener") };
    const jmethodID method_notifyUnifiedProgress { ::djinni::jniGetMethodID(clazz.get(), "notifyUnifiedProgress", "(F)V") };
    const jmethodID method_notifyBalanceChange { ::djinni::jniGetMethodID(clazz.get(), "notifyBalanceChange", "(Lcom/novo/jniunifiedbackend/BalanceRecord;)V") };
    const jmethodID method_notifyNewMutation { ::djinni::jniGetMethodID(clazz.get(), "notifyNewMutation", "(Lcom/novo/jniunifiedbackend/MutationRecord;Z)V") };
    const jmethodID method_notifyUpdatedTransaction { ::djinni::jniGetMethodID(clazz.get(), "notifyUpdatedTransaction", "(Lcom/novo/jniunifiedbackend/TransactionRecord;)V") };
    const jmethodID method_notifyInitWithExistingWallet { ::djinni::jniGetMethodID(clazz.get(), "notifyInitWithExistingWallet", "()V") };
    const jmethodID method_notifyInitWithoutExistingWallet { ::djinni::jniGetMethodID(clazz.get(), "notifyInitWithoutExistingWallet", "()V") };
    const jmethodID method_notifyShutdown { ::djinni::jniGetMethodID(clazz.get(), "notifyShutdown", "()V") };
    const jmethodID method_notifyCoreReady { ::djinni::jniGetMethodID(clazz.get(), "notifyCoreReady", "()V") };
    const jmethodID method_logPrint { ::djinni::jniGetMethodID(clazz.get(), "logPrint", "(Ljava/lang/String;)V") };
};

}  // namespace djinni_generated
