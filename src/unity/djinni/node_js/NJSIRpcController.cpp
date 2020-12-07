// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NJSIRpcController.hpp"
using namespace std;

void NJSIRpcController::execute(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 2)
    {
        Napi::Error::New(env, "NJSIRpcController::execute needs 2 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();
    std::shared_ptr<NJSIRpcListener> arg_1(std::shared_ptr<NJSIRpcListener>{}, NJSIRpcListener::Unwrap(info[1].As<Napi::Object>()));

    try
    {
        IRpcController::execute(arg_0,arg_1);
    }
    catch (std::exception& e)
    {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return;
    }
    catch (...)
    {
        Napi::Error::New(env, "core exception thrown").ThrowAsJavaScriptException();
        return;
    }
}
Napi::Value NJSIRpcController::getAutocompleteList(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIRpcController::getAutocompleteList needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types

    try
    {
        auto result = IRpcController::getAutocompleteList();

        //Wrap result in node object
        auto arg_0 = Napi::Array::New(env);
        for(size_t arg_0_id = 0; arg_0_id < result.size(); arg_0_id++)
        {
            auto arg_0_elem = Napi::String::New(env, result[arg_0_id]);
            arg_0.Set((int)arg_0_id,arg_0_elem);
        }


        return arg_0;
    }
    catch (std::exception& e)
    {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return Napi::Value();
    }
    catch (...)
    {
        Napi::Error::New(env, "core exception thrown").ThrowAsJavaScriptException();
        return Napi::Value();
    }
}

Napi::FunctionReference NJSIRpcController::constructor;

Napi::Object NJSIRpcController::Init(Napi::Env env, Napi::Object exports) {

    // Hook all method callbacks
    Napi::Function func = DefineClass(env, "NJSIRpcController", {
    InstanceMethod("execute", &NJSIRpcController::execute),
    InstanceMethod("getAutocompleteList", &NJSIRpcController::getAutocompleteList),
    });
    // Create a peristent reference to the class constructor. This will allow a function called on a class prototype and a function called on instance of a class to be distinguished from each other.
    constructor = Napi::Persistent(func);
    // Call the SuppressDestruct() method on the static data prevent the calling to this destructor to reset the reference when the environment is no longer available.
    constructor.SuppressDestruct();
    exports.Set("NJSIRpcController", func);
    return exports;
}
