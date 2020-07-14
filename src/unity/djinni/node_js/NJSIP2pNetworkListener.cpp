// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NJSIP2pNetworkListener.hpp"
using namespace std;

void NJSIP2pNetworkListener::onNetworkEnabled_aimpl__()
{
    const auto& env = Env();
    Napi::HandleScope scope(env);
    //Wrap parameters
    std::vector<napi_value> args;
    Napi::Value calling_function_as_value = Value().Get("onNetworkEnabled");
    if(!calling_function_as_value.IsUndefined() && !calling_function_as_value.IsNull())
    {
        Napi::Function calling_function = calling_function_as_value.As<Napi::Function>();
        auto result_onNetworkEnabled = calling_function.Call(args);
        if(result_onNetworkEnabled.IsEmpty())
        {
            Napi::Error::New(env, "NJSIP2pNetworkListener::onNetworkEnabled call failed").ThrowAsJavaScriptException();
            return;
        }
    }
}

void NJSIP2pNetworkListener::onNetworkEnabled()
{
    uv_work_t* request = new uv_work_t;
    request->data = new std::tuple<NJSIP2pNetworkListener*>(this);

    uv_queue_work(uv_default_loop(), request, [](uv_work_t*) -> void{}, [](uv_work_t* req, int status) -> void
    {
        NJSIP2pNetworkListener* pthis = std::get<0>(*((std::tuple<NJSIP2pNetworkListener*>*)req->data));
        pthis->onNetworkEnabled_aimpl__();
        delete (std::tuple<NJSIP2pNetworkListener*>*)req->data;
        req->data = nullptr;
    }
    );
}

void NJSIP2pNetworkListener::onNetworkDisabled_aimpl__()
{
    const auto& env = Env();
    Napi::HandleScope scope(env);
    //Wrap parameters
    std::vector<napi_value> args;
    Napi::Value calling_function_as_value = Value().Get("onNetworkDisabled");
    if(!calling_function_as_value.IsUndefined() && !calling_function_as_value.IsNull())
    {
        Napi::Function calling_function = calling_function_as_value.As<Napi::Function>();
        auto result_onNetworkDisabled = calling_function.Call(args);
        if(result_onNetworkDisabled.IsEmpty())
        {
            Napi::Error::New(env, "NJSIP2pNetworkListener::onNetworkDisabled call failed").ThrowAsJavaScriptException();
            return;
        }
    }
}

void NJSIP2pNetworkListener::onNetworkDisabled()
{
    uv_work_t* request = new uv_work_t;
    request->data = new std::tuple<NJSIP2pNetworkListener*>(this);

    uv_queue_work(uv_default_loop(), request, [](uv_work_t*) -> void{}, [](uv_work_t* req, int status) -> void
    {
        NJSIP2pNetworkListener* pthis = std::get<0>(*((std::tuple<NJSIP2pNetworkListener*>*)req->data));
        pthis->onNetworkDisabled_aimpl__();
        delete (std::tuple<NJSIP2pNetworkListener*>*)req->data;
        req->data = nullptr;
    }
    );
}

void NJSIP2pNetworkListener::onConnectionCountChanged_aimpl__(int32_t numConnections)
{
    const auto& env = Env();
    Napi::HandleScope scope(env);
    //Wrap parameters
    std::vector<napi_value> args;
    auto arg_0 = Napi::Number::New(env, numConnections);
    args.push_back(arg_0);
    Napi::Value calling_function_as_value = Value().Get("onConnectionCountChanged");
    if(!calling_function_as_value.IsUndefined() && !calling_function_as_value.IsNull())
    {
        Napi::Function calling_function = calling_function_as_value.As<Napi::Function>();
        auto result_onConnectionCountChanged = calling_function.Call(args);
        if(result_onConnectionCountChanged.IsEmpty())
        {
            Napi::Error::New(env, "NJSIP2pNetworkListener::onConnectionCountChanged call failed").ThrowAsJavaScriptException();
            return;
        }
    }
}

void NJSIP2pNetworkListener::onConnectionCountChanged(int32_t numConnections)
{
    uv_work_t* request = new uv_work_t;
    request->data = new std::tuple<NJSIP2pNetworkListener*, int32_t>(this, numConnections);

    uv_queue_work(uv_default_loop(), request, [](uv_work_t*) -> void{}, [](uv_work_t* req, int status) -> void
    {
        NJSIP2pNetworkListener* pthis = std::get<0>(*((std::tuple<NJSIP2pNetworkListener*, int32_t>*)req->data));
        pthis->onConnectionCountChanged_aimpl__(std::get<1>(*((std::tuple<NJSIP2pNetworkListener*, int32_t>*)req->data)));
        delete (std::tuple<NJSIP2pNetworkListener*, int32_t>*)req->data;
        req->data = nullptr;
    }
    );
}

void NJSIP2pNetworkListener::onBytesChanged_aimpl__(int32_t totalRecv, int32_t totalSent)
{
    const auto& env = Env();
    Napi::HandleScope scope(env);
    //Wrap parameters
    std::vector<napi_value> args;
    auto arg_0 = Napi::Number::New(env, totalRecv);
    args.push_back(arg_0);
    auto arg_1 = Napi::Number::New(env, totalSent);
    args.push_back(arg_1);
    Napi::Value calling_function_as_value = Value().Get("onBytesChanged");
    if(!calling_function_as_value.IsUndefined() && !calling_function_as_value.IsNull())
    {
        Napi::Function calling_function = calling_function_as_value.As<Napi::Function>();
        auto result_onBytesChanged = calling_function.Call(args);
        if(result_onBytesChanged.IsEmpty())
        {
            Napi::Error::New(env, "NJSIP2pNetworkListener::onBytesChanged call failed").ThrowAsJavaScriptException();
            return;
        }
    }
}

void NJSIP2pNetworkListener::onBytesChanged(int32_t totalRecv, int32_t totalSent)
{
    uv_work_t* request = new uv_work_t;
    request->data = new std::tuple<NJSIP2pNetworkListener*, int32_t, int32_t>(this, totalRecv, totalSent);

    uv_queue_work(uv_default_loop(), request, [](uv_work_t*) -> void{}, [](uv_work_t* req, int status) -> void
    {
        NJSIP2pNetworkListener* pthis = std::get<0>(*((std::tuple<NJSIP2pNetworkListener*, int32_t, int32_t>*)req->data));
        pthis->onBytesChanged_aimpl__(std::get<1>(*((std::tuple<NJSIP2pNetworkListener*, int32_t, int32_t>*)req->data)), std::get<2>(*((std::tuple<NJSIP2pNetworkListener*, int32_t, int32_t>*)req->data)));
        delete (std::tuple<NJSIP2pNetworkListener*, int32_t, int32_t>*)req->data;
        req->data = nullptr;
    }
    );
}

Napi::FunctionReference NJSIP2pNetworkListener::constructor;

Napi::Object NJSIP2pNetworkListener::Init(Napi::Env env, Napi::Object exports) {

    Napi::Function func = DefineClass(env, "NJSIP2pNetworkListener",{});
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("NJSIP2pNetworkListener", func);
    return exports;
}
