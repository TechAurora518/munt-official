// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include "djinni_support.hpp"
#include "witness_funding_result_record.hpp"

namespace djinni_generated {

class NativeWitnessFundingResultRecord final {
public:
    using CppType = ::WitnessFundingResultRecord;
    using JniType = jobject;

    using Boxed = NativeWitnessFundingResultRecord;

    ~NativeWitnessFundingResultRecord();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    NativeWitnessFundingResultRecord();
    friend ::djinni::JniClass<NativeWitnessFundingResultRecord>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("unity_wallet/jniunifiedbackend/WitnessFundingResultRecord") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;J)V") };
    const jfieldID field_mStatus { ::djinni::jniGetFieldID(clazz.get(), "mStatus", "Ljava/lang/String;") };
    const jfieldID field_mTxid { ::djinni::jniGetFieldID(clazz.get(), "mTxid", "Ljava/lang/String;") };
    const jfieldID field_mFee { ::djinni::jniGetFieldID(clazz.get(), "mFee", "J") };
};

}  // namespace djinni_generated
