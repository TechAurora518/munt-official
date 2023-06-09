// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NativeBannedPeerRecord.hpp"  // my header
#include "Marshal.hpp"

namespace djinni_generated {

NativeBannedPeerRecord::NativeBannedPeerRecord() = default;

NativeBannedPeerRecord::~NativeBannedPeerRecord() = default;

auto NativeBannedPeerRecord::fromCpp(JNIEnv* jniEnv, const CppType& c) -> ::djinni::LocalRef<JniType> {
    const auto& data = ::djinni::JniClass<NativeBannedPeerRecord>::get();
    auto r = ::djinni::LocalRef<JniType>{jniEnv->NewObject(data.clazz.get(), data.jconstructor,
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.address)),
                                                           ::djinni::get(::djinni::I64::fromCpp(jniEnv, c.banned_until)),
                                                           ::djinni::get(::djinni::I64::fromCpp(jniEnv, c.banned_from)),
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.reason)))};
    ::djinni::jniExceptionCheck(jniEnv);
    return r;
}

auto NativeBannedPeerRecord::toCpp(JNIEnv* jniEnv, JniType j) -> CppType {
    ::djinni::JniLocalScope jscope(jniEnv, 5);
    assert(j != nullptr);
    const auto& data = ::djinni::JniClass<NativeBannedPeerRecord>::get();
    return {::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_mAddress)),
            ::djinni::I64::toCpp(jniEnv, jniEnv->GetLongField(j, data.field_mBannedUntil)),
            ::djinni::I64::toCpp(jniEnv, jniEnv->GetLongField(j, data.field_mBannedFrom)),
            ::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_mReason))};
}

}  // namespace djinni_generated
