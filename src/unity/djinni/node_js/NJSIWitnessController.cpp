// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NJSIWitnessController.hpp"
using namespace std;

Napi::Value NJSIWitnessController::getNetworkLimits(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIWitnessController::getNetworkLimits needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types

    auto result = IWitnessController::getNetworkLimits();

    //Wrap result in node object
    auto arg_0 = Napi::Object::New(env);
    for(auto const& arg_0_elem : result)
    {
        auto arg_0_first = Napi::String::New(env, arg_0_elem.first);
        auto arg_0_second = Napi::String::New(env, arg_0_elem.second);
        arg_0.Set(arg_0_first, arg_0_second);
    }


    return arg_0;
}

Napi::FunctionReference NJSIWitnessController::constructor;

Napi::Object NJSIWitnessController::Init(Napi::Env env, Napi::Object exports) {

    // Hook all method callbacks
    Napi::Function func = DefineClass(env, "NJSIWitnessController", {
    InstanceMethod("getNetworkLimits", &NJSIWitnessController::getNetworkLimits),
    });
    // Create a peristent reference to the class constructor. This will allow a function called on a class prototype and a function called on instance of a class to be distinguished from each other.
    constructor = Napi::Persistent(func);
    // Call the SuppressDestruct() method on the static data prevent the calling to this destructor to reset the reference when the environment is no longer available.
    constructor.SuppressDestruct();
    exports.Set("NJSIWitnessController", func);
    return exports;
}
