// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NJSIAccountsController.hpp"
using namespace std;

void NJSIAccountsController::setListener(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIAccountsController::setListener needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::shared_ptr<NJSIAccountsListener> arg_0(std::shared_ptr<NJSIAccountsListener>{}, NJSIAccountsListener::Unwrap(info[0].As<Napi::Object>()));

    IAccountsController::setListener(arg_0);
}
Napi::Value NJSIAccountsController::setActiveAccount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIAccountsController::setActiveAccount needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();

    auto result = IAccountsController::setActiveAccount(arg_0);

    //Wrap result in node object
    auto arg_1 = Napi::Boolean::New(env, result);

    return arg_1;
}
Napi::Value NJSIAccountsController::createAccount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 2)
    {
        Napi::Error::New(env, "NJSIAccountsController::createAccount needs 2 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();
    std::string arg_1 = info[1].As<Napi::String>();

    auto result = IAccountsController::createAccount(arg_0,arg_1);

    //Wrap result in node object
    auto arg_2 = Napi::String::New(env, result);

    return arg_2;
}
Napi::Value NJSIAccountsController::renameAccount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 2)
    {
        Napi::Error::New(env, "NJSIAccountsController::renameAccount needs 2 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();
    std::string arg_1 = info[1].As<Napi::String>();

    auto result = IAccountsController::renameAccount(arg_0,arg_1);

    //Wrap result in node object
    auto arg_2 = Napi::Boolean::New(env, result);

    return arg_2;
}
Napi::Value NJSIAccountsController::getAccountLinkURI(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIAccountsController::getAccountLinkURI needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();

    auto result = IAccountsController::getAccountLinkURI(arg_0);

    //Wrap result in node object
    auto arg_1 = Napi::String::New(env, result);

    return arg_1;
}
Napi::Value NJSIAccountsController::getWitnessKeyURI(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIAccountsController::getWitnessKeyURI needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();

    auto result = IAccountsController::getWitnessKeyURI(arg_0);

    //Wrap result in node object
    auto arg_1 = Napi::String::New(env, result);

    return arg_1;
}
Napi::Value NJSIAccountsController::deleteAccount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIAccountsController::deleteAccount needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();

    auto result = IAccountsController::deleteAccount(arg_0);

    //Wrap result in node object
    auto arg_1 = Napi::Boolean::New(env, result);

    return arg_1;
}
Napi::Value NJSIAccountsController::purgeAccount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIAccountsController::purgeAccount needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();

    auto result = IAccountsController::purgeAccount(arg_0);

    //Wrap result in node object
    auto arg_1 = Napi::Boolean::New(env, result);

    return arg_1;
}
Napi::Value NJSIAccountsController::listAccounts(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIAccountsController::listAccounts needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types

    auto result = IAccountsController::listAccounts();

    //Wrap result in node object
    auto arg_0 = Napi::Array::New(env);
    for(size_t arg_0_id = 0; arg_0_id < result.size(); arg_0_id++)
    {
        auto arg_0_elem = Napi::Object::New(env);
        auto arg_0_elem_1 = Napi::String::New(env, result[arg_0_id].UUID);
        arg_0_elem.Set("UUID", arg_0_elem_1);
        auto arg_0_elem_2 = Napi::String::New(env, result[arg_0_id].label);
        arg_0_elem.Set("label", arg_0_elem_2);
        auto arg_0_elem_3 = Napi::String::New(env, result[arg_0_id].state);
        arg_0_elem.Set("state", arg_0_elem_3);
        auto arg_0_elem_4 = Napi::String::New(env, result[arg_0_id].type);
        arg_0_elem.Set("type", arg_0_elem_4);
        auto arg_0_elem_5 = Napi::Boolean::New(env, result[arg_0_id].isHD);
        arg_0_elem.Set("isHD", arg_0_elem_5);

        arg_0.Set((int)arg_0_id,arg_0_elem);
    }


    return arg_0;
}

Napi::FunctionReference NJSIAccountsController::constructor;

Napi::Object NJSIAccountsController::Init(Napi::Env env, Napi::Object exports) {

    // Hook all method callbacks
    Napi::Function func = DefineClass(env, "NJSIAccountsController", {
    InstanceMethod("setListener", &NJSIAccountsController::setListener),
    InstanceMethod("setActiveAccount", &NJSIAccountsController::setActiveAccount),
    InstanceMethod("createAccount", &NJSIAccountsController::createAccount),
    InstanceMethod("renameAccount", &NJSIAccountsController::renameAccount),
    InstanceMethod("getAccountLinkURI", &NJSIAccountsController::getAccountLinkURI),
    InstanceMethod("getWitnessKeyURI", &NJSIAccountsController::getWitnessKeyURI),
    InstanceMethod("deleteAccount", &NJSIAccountsController::deleteAccount),
    InstanceMethod("purgeAccount", &NJSIAccountsController::purgeAccount),
    InstanceMethod("listAccounts", &NJSIAccountsController::listAccounts),
    });
    // Create a peristent reference to the class constructor. This will allow a function called on a class prototype and a function called on instance of a class to be distinguished from each other.
    constructor = Napi::Persistent(func);
    // Call the SuppressDestruct() method on the static data prevent the calling to this destructor to reset the reference when the environment is no longer available.
    constructor.SuppressDestruct();
    exports.Set("NJSIAccountsController", func);
    return exports;
}
