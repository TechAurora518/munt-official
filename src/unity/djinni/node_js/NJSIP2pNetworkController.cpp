// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NJSIP2pNetworkController.hpp"
using namespace std;

void NJSIP2pNetworkController::setListener(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::setListener needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::shared_ptr<NJSIP2pNetworkListener> arg_0(std::shared_ptr<NJSIP2pNetworkListener>{}, NJSIP2pNetworkListener::Unwrap(info[0].As<Napi::Object>()));

    try
    {
        IP2pNetworkController::setListener(arg_0);
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
void NJSIP2pNetworkController::disableNetwork(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::disableNetwork needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    try
    {
        IP2pNetworkController::disableNetwork();
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
void NJSIP2pNetworkController::enableNetwork(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::enableNetwork needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    try
    {
        IP2pNetworkController::enableNetwork();
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
Napi::Value NJSIP2pNetworkController::getPeerInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::getPeerInfo needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types

    try
    {
        auto result = IP2pNetworkController::getPeerInfo();

        //Wrap result in node object
        auto arg_0 = Napi::Array::New(env);
        for(size_t arg_0_id = 0; arg_0_id < result.size(); arg_0_id++)
        {
            auto arg_0_elem = Napi::Object::New(env);
            auto arg_0_elem_1 = Napi::Value::From(env, result[arg_0_id].id);
            arg_0_elem.Set("id", arg_0_elem_1);
            auto arg_0_elem_2 = Napi::String::New(env, result[arg_0_id].ip);
            arg_0_elem.Set("ip", arg_0_elem_2);
            auto arg_0_elem_3 = Napi::String::New(env, result[arg_0_id].hostname);
            arg_0_elem.Set("hostname", arg_0_elem_3);
            auto arg_0_elem_4 = Napi::String::New(env, result[arg_0_id].addrLocal);
            arg_0_elem.Set("addrLocal", arg_0_elem_4);
            auto arg_0_elem_5 = Napi::String::New(env, result[arg_0_id].addrBind);
            arg_0_elem.Set("addrBind", arg_0_elem_5);
            auto arg_0_elem_6 = Napi::Value::From(env, result[arg_0_id].start_height);
            arg_0_elem.Set("start_height", arg_0_elem_6);
            auto arg_0_elem_7 = Napi::Value::From(env, result[arg_0_id].synced_height);
            arg_0_elem.Set("synced_height", arg_0_elem_7);
            auto arg_0_elem_8 = Napi::Value::From(env, result[arg_0_id].common_height);
            arg_0_elem.Set("common_height", arg_0_elem_8);
            auto arg_0_elem_9 = Napi::Value::From(env, result[arg_0_id].time_connected);
            arg_0_elem.Set("time_connected", arg_0_elem_9);
            auto arg_0_elem_10 = Napi::Value::From(env, result[arg_0_id].time_offset);
            arg_0_elem.Set("time_offset", arg_0_elem_10);
            auto arg_0_elem_11 = Napi::Value::From(env, result[arg_0_id].latency);
            arg_0_elem.Set("latency", arg_0_elem_11);
            auto arg_0_elem_12 = Napi::Value::From(env, result[arg_0_id].last_send);
            arg_0_elem.Set("last_send", arg_0_elem_12);
            auto arg_0_elem_13 = Napi::Value::From(env, result[arg_0_id].last_receive);
            arg_0_elem.Set("last_receive", arg_0_elem_13);
            auto arg_0_elem_14 = Napi::Value::From(env, result[arg_0_id].send_bytes);
            arg_0_elem.Set("send_bytes", arg_0_elem_14);
            auto arg_0_elem_15 = Napi::Value::From(env, result[arg_0_id].receive_bytes);
            arg_0_elem.Set("receive_bytes", arg_0_elem_15);
            auto arg_0_elem_16 = Napi::String::New(env, result[arg_0_id].userAgent);
            arg_0_elem.Set("userAgent", arg_0_elem_16);
            auto arg_0_elem_17 = Napi::Value::From(env, result[arg_0_id].protocol);
            arg_0_elem.Set("protocol", arg_0_elem_17);
            auto arg_0_elem_18 = Napi::Value::From(env, result[arg_0_id].services);
            arg_0_elem.Set("services", arg_0_elem_18);
            auto arg_0_elem_19 = Napi::Value::From(env, result[arg_0_id].inbound);
            arg_0_elem.Set("inbound", arg_0_elem_19);
            auto arg_0_elem_20 = Napi::Value::From(env, result[arg_0_id].whitelisted);
            arg_0_elem.Set("whitelisted", arg_0_elem_20);
            auto arg_0_elem_21 = Napi::Value::From(env, result[arg_0_id].addnode);
            arg_0_elem.Set("addnode", arg_0_elem_21);
            auto arg_0_elem_22 = Napi::Value::From(env, result[arg_0_id].relay_txes);
            arg_0_elem.Set("relay_txes", arg_0_elem_22);
            auto arg_0_elem_23 = Napi::Value::From(env, result[arg_0_id].banscore);
            arg_0_elem.Set("banscore", arg_0_elem_23);

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
Napi::Value NJSIP2pNetworkController::listBannedPeers(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::listBannedPeers needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types

    try
    {
        auto result = IP2pNetworkController::listBannedPeers();

        //Wrap result in node object
        auto arg_0 = Napi::Array::New(env);
        for(size_t arg_0_id = 0; arg_0_id < result.size(); arg_0_id++)
        {
            auto arg_0_elem = Napi::Object::New(env);
            auto arg_0_elem_1 = Napi::String::New(env, result[arg_0_id].address);
            arg_0_elem.Set("address", arg_0_elem_1);
            auto arg_0_elem_2 = Napi::Value::From(env, result[arg_0_id].banned_until);
            arg_0_elem.Set("banned_until", arg_0_elem_2);
            auto arg_0_elem_3 = Napi::Value::From(env, result[arg_0_id].banned_from);
            arg_0_elem.Set("banned_from", arg_0_elem_3);
            auto arg_0_elem_4 = Napi::String::New(env, result[arg_0_id].reason);
            arg_0_elem.Set("reason", arg_0_elem_4);

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
Napi::Value NJSIP2pNetworkController::banPeer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 2)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::banPeer needs 2 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();
    auto arg_1 = info[1].ToNumber().Int64Value();

    try
    {
        auto result = IP2pNetworkController::banPeer(arg_0,arg_1);

        //Wrap result in node object
        auto arg_2 = Napi::Value::From(env, result);

        return arg_2;
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
Napi::Value NJSIP2pNetworkController::unbanPeer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::unbanPeer needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    std::string arg_0 = info[0].As<Napi::String>();

    try
    {
        auto result = IP2pNetworkController::unbanPeer(arg_0);

        //Wrap result in node object
        auto arg_1 = Napi::Value::From(env, result);

        return arg_1;
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
Napi::Value NJSIP2pNetworkController::disconnectPeer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 1)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::disconnectPeer needs 1 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types
    auto arg_0 = info[0].ToNumber().Int64Value();

    try
    {
        auto result = IP2pNetworkController::disconnectPeer(arg_0);

        //Wrap result in node object
        auto arg_1 = Napi::Value::From(env, result);

        return arg_1;
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
Napi::Value NJSIP2pNetworkController::ClearBanned(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();


    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        Napi::Error::New(env, "NJSIP2pNetworkController::ClearBanned needs 0 arguments").ThrowAsJavaScriptException();
    }

    //Check if parameters have correct types

    try
    {
        auto result = IP2pNetworkController::ClearBanned();

        //Wrap result in node object
        auto arg_0 = Napi::Value::From(env, result);

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

Napi::FunctionReference NJSIP2pNetworkController::constructor;

Napi::Object NJSIP2pNetworkController::Init(Napi::Env env, Napi::Object exports) {

    // Hook all method callbacks
    Napi::Function func = DefineClass(env, "NJSIP2pNetworkController", {
    InstanceMethod("setListener", &NJSIP2pNetworkController::setListener),
    InstanceMethod("disableNetwork", &NJSIP2pNetworkController::disableNetwork),
    InstanceMethod("enableNetwork", &NJSIP2pNetworkController::enableNetwork),
    InstanceMethod("getPeerInfo", &NJSIP2pNetworkController::getPeerInfo),
    InstanceMethod("listBannedPeers", &NJSIP2pNetworkController::listBannedPeers),
    InstanceMethod("banPeer", &NJSIP2pNetworkController::banPeer),
    InstanceMethod("unbanPeer", &NJSIP2pNetworkController::unbanPeer),
    InstanceMethod("disconnectPeer", &NJSIP2pNetworkController::disconnectPeer),
    InstanceMethod("ClearBanned", &NJSIP2pNetworkController::ClearBanned),
    });
    // Create a peristent reference to the class constructor. This will allow a function called on a class prototype and a function called on instance of a class to be distinguished from each other.
    constructor = Napi::Persistent(func);
    // Call the SuppressDestruct() method on the static data prevent the calling to this destructor to reset the reference when the environment is no longer available.
    constructor.SuppressDestruct();
    exports.Set("NJSIP2pNetworkController", func);
    return exports;
}
