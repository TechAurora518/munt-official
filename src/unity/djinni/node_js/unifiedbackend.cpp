// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni

#include <napi.h>

#include "NJSUnifiedBackendCpp.hpp"
#include "NJSUnifiedFrontend.hpp"
#include "NJSMonitorListener.hpp"


Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
    NJSUnifiedBackend::Init(env, exports);
    NJSUnifiedFrontend::Init(env, exports);
    NJSMonitorListener::Init(env, exports);
    return exports;
}
NODE_API_MODULE(unifiedbackend,InitAll);
