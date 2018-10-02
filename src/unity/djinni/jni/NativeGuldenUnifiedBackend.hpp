// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include "djinni_support.hpp"
#include "gulden_unified_backend.hpp"

namespace djinni_generated {

class NativeGuldenUnifiedBackend final : ::djinni::JniInterface<::GuldenUnifiedBackend, NativeGuldenUnifiedBackend> {
public:
    using CppType = std::shared_ptr<::GuldenUnifiedBackend>;
    using CppOptType = std::shared_ptr<::GuldenUnifiedBackend>;
    using JniType = jobject;

    using Boxed = NativeGuldenUnifiedBackend;

    ~NativeGuldenUnifiedBackend();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<NativeGuldenUnifiedBackend>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<NativeGuldenUnifiedBackend>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    NativeGuldenUnifiedBackend();
    friend ::djinni::JniClass<NativeGuldenUnifiedBackend>;
    friend ::djinni::JniInterface<::GuldenUnifiedBackend, NativeGuldenUnifiedBackend>;

};

}  // namespace djinni_generated
