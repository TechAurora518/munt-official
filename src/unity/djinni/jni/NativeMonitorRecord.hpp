// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include "djinni_support.hpp"
#include "monitor_record.hpp"

namespace djinni_generated {

class NativeMonitorRecord final {
public:
    using CppType = ::MonitorRecord;
    using JniType = jobject;

    using Boxed = NativeMonitorRecord;

    ~NativeMonitorRecord();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    NativeMonitorRecord();
    friend ::djinni::JniClass<NativeMonitorRecord>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("unity_wallet/jniunifiedbackend/MonitorRecord") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(IIIII)V") };
    const jfieldID field_mPartialHeight { ::djinni::jniGetFieldID(clazz.get(), "mPartialHeight", "I") };
    const jfieldID field_mPartialOffset { ::djinni::jniGetFieldID(clazz.get(), "mPartialOffset", "I") };
    const jfieldID field_mPrunedHeight { ::djinni::jniGetFieldID(clazz.get(), "mPrunedHeight", "I") };
    const jfieldID field_mProcessedSPVHeight { ::djinni::jniGetFieldID(clazz.get(), "mProcessedSPVHeight", "I") };
    const jfieldID field_mProbableHeight { ::djinni::jniGetFieldID(clazz.get(), "mProbableHeight", "I") };
};

}  // namespace djinni_generated
