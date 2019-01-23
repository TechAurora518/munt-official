// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NativeTransactionRecord.hpp"  // my header
#include "Marshal.hpp"
#include "NativeOutputRecord.hpp"

namespace djinni_generated {

NativeTransactionRecord::NativeTransactionRecord() = default;

NativeTransactionRecord::~NativeTransactionRecord() = default;

auto NativeTransactionRecord::fromCpp(JNIEnv* jniEnv, const CppType& c) -> ::djinni::LocalRef<JniType> {
    const auto& data = ::djinni::JniClass<NativeTransactionRecord>::get();
    auto r = ::djinni::LocalRef<JniType>{jniEnv->NewObject(data.clazz.get(), data.jconstructor,
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.txHash)),
                                                           ::djinni::get(::djinni::I64::fromCpp(jniEnv, c.timestamp)),
                                                           ::djinni::get(::djinni::I64::fromCpp(jniEnv, c.amount)),
                                                           ::djinni::get(::djinni::I64::fromCpp(jniEnv, c.fee)),
                                                           ::djinni::get(::djinni::List<::djinni_generated::NativeOutputRecord>::fromCpp(jniEnv, c.receivedOutputs)),
                                                           ::djinni::get(::djinni::List<::djinni_generated::NativeOutputRecord>::fromCpp(jniEnv, c.sentOutputs)))};
    ::djinni::jniExceptionCheck(jniEnv);
    return r;
}

auto NativeTransactionRecord::toCpp(JNIEnv* jniEnv, JniType j) -> CppType {
    ::djinni::JniLocalScope jscope(jniEnv, 7);
    assert(j != nullptr);
    const auto& data = ::djinni::JniClass<NativeTransactionRecord>::get();
    return {::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_mTxHash)),
            ::djinni::I64::toCpp(jniEnv, jniEnv->GetLongField(j, data.field_mTimestamp)),
            ::djinni::I64::toCpp(jniEnv, jniEnv->GetLongField(j, data.field_mAmount)),
            ::djinni::I64::toCpp(jniEnv, jniEnv->GetLongField(j, data.field_mFee)),
            ::djinni::List<::djinni_generated::NativeOutputRecord>::toCpp(jniEnv, jniEnv->GetObjectField(j, data.field_mReceivedOutputs)),
            ::djinni::List<::djinni_generated::NativeOutputRecord>::toCpp(jniEnv, jniEnv->GetObjectField(j, data.field_mSentOutputs))};
}

}  // namespace djinni_generated
