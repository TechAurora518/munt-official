// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#ifndef DJINNI_GENERATED_NJSIRPCLISTENER_HPP
#define DJINNI_GENERATED_NJSIRPCLISTENER_HPP


#include <string>

#include <napi.h>
#include <uv.h>
#include <i_rpc_listener.hpp>

using namespace std;

class NJSIRpcListener: public Napi::ObjectWrap<NJSIRpcListener> {
public:

    static Napi::FunctionReference constructor;
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NJSIRpcListener(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NJSIRpcListener>(info){};

    /**
     * Returns a filtered version of the command with sensitive information like passwords removed
     * Any kind of 'command history' functionality should store this filtered command and not the original command
     */
    void onFilteredCommand(const std::string & filteredCommand);

    /**
     * Returns the result and a filtered version of the command with sensitive information like passwords removed
     * Any kind of 'command history' functionality should store this filtered command and not the original command
     */
    void onSuccess(const std::string & filteredCommand, const std::string & result);

    /** Returns an error message which might be a plain string or JSON depending on the type of error */
    void onError(const std::string & errorMessage);

private:
    /**
     * Returns a filtered version of the command with sensitive information like passwords removed
     * Any kind of 'command history' functionality should store this filtered command and not the original command
     */
    void onFilteredCommand(const Napi::CallbackInfo& info);
    void onFilteredCommand_aimpl__(const std::string & filteredCommand);

    /**
     * Returns the result and a filtered version of the command with sensitive information like passwords removed
     * Any kind of 'command history' functionality should store this filtered command and not the original command
     */
    void onSuccess(const Napi::CallbackInfo& info);
    void onSuccess_aimpl__(const std::string & filteredCommand, const std::string & result);

    /** Returns an error message which might be a plain string or JSON depending on the type of error */
    void onError(const Napi::CallbackInfo& info);
    void onError_aimpl__(const std::string & errorMessage);

};
#endif //DJINNI_GENERATED_NJSIRPCLISTENER_HPP
