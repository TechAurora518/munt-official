// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#ifndef DJINNI_GENERATED_NJSIACCOUNTSLISTENER_HPP
#define DJINNI_GENERATED_NJSIACCOUNTSLISTENER_HPP


#include "account_link_record.hpp"
#include "account_record.hpp"
#include <string>

#include <napi.h>
#include <uv.h>
#include <i_accounts_listener.hpp>

using namespace std;

class NJSIAccountsListener: public Napi::ObjectWrap<NJSIAccountsListener> {
public:

    static Napi::FunctionReference constructor;
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NJSIAccountsListener(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NJSIAccountsListener>(info){};

    /** Notify that the active account has changed */
    void onActiveAccountChanged(const std::string & accountUUID);

    /** Notify that the active account name has changed */
    void onActiveAccountNameChanged(const std::string & newAccountName);

    /** Notify that an account name has changed */
    void onAccountNameChanged(const std::string & accountUUID, const std::string & newAccountName);

    /** Notify that a new account has been added */
    void onAccountAdded(const std::string & accountUUID, const std::string & accountName);

    /** Notify that an account has been deleted */
    void onAccountDeleted(const std::string & accountUUID);

    /** Notify that an account has been modified */
    void onAccountModified(const std::string & accountUUID, const AccountRecord & accountData);

private:
    /** Notify that the active account has changed */
    void onActiveAccountChanged(const Napi::CallbackInfo& info);
    void onActiveAccountChanged_aimpl__(const std::string & accountUUID);

    /** Notify that the active account name has changed */
    void onActiveAccountNameChanged(const Napi::CallbackInfo& info);
    void onActiveAccountNameChanged_aimpl__(const std::string & newAccountName);

    /** Notify that an account name has changed */
    void onAccountNameChanged(const Napi::CallbackInfo& info);
    void onAccountNameChanged_aimpl__(const std::string & accountUUID, const std::string & newAccountName);

    /** Notify that a new account has been added */
    void onAccountAdded(const Napi::CallbackInfo& info);
    void onAccountAdded_aimpl__(const std::string & accountUUID, const std::string & accountName);

    /** Notify that an account has been deleted */
    void onAccountDeleted(const Napi::CallbackInfo& info);
    void onAccountDeleted_aimpl__(const std::string & accountUUID);

    /** Notify that an account has been modified */
    void onAccountModified(const Napi::CallbackInfo& info);
    void onAccountModified_aimpl__(const std::string & accountUUID, const AccountRecord & accountData);

};
#endif //DJINNI_GENERATED_NJSIACCOUNTSLISTENER_HPP
