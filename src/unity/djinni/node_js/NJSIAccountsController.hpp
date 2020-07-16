// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#ifndef DJINNI_GENERATED_NJSIACCOUNTSCONTROLLER_HPP
#define DJINNI_GENERATED_NJSIACCOUNTSCONTROLLER_HPP


#include "NJSIAccountsListener.hpp"
#include "account_record.hpp"
#include <memory>
#include <string>
#include <vector>

#include <napi.h>
#include <uv.h>
#include <i_accounts_controller.hpp>

using namespace std;

class NJSIAccountsController: public Napi::ObjectWrap<NJSIAccountsController> {
public:

    static Napi::FunctionReference constructor;
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NJSIAccountsController(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NJSIAccountsController>(info){};

private:
    /** Register listener to be notified of account related events */
    void setListener(const Napi::CallbackInfo& info);

    /** Set the currently active account */
    Napi::Value setActiveAccount(const Napi::CallbackInfo& info);

    /** Create an account, possible types are (HD/Mobile/Witness/Mining/Legacy). Returns the UUID of the new account */
    Napi::Value createAccount(const Napi::CallbackInfo& info);

    /** Rename an account */
    Napi::Value renameAccount(const Napi::CallbackInfo& info);

    /** Get a URI that will enable 'linking' of this account in another wallet (for e.g. mobile wallet linking) for an account. Empty on failiure.  */
    Napi::Value getAccountLinkURI(const Napi::CallbackInfo& info);

    /** Get a URI that will enable creation of a "witness only" account in another wallet that can witness on behalf of this account */
    Napi::Value getWitnessKeyURI(const Napi::CallbackInfo& info);

    /** Delete an account, account remains available in background but is hidden from user */
    Napi::Value deleteAccount(const Napi::CallbackInfo& info);

    /**
     * Purge an account, account is permenently removed from wallet (but may still reappear in some instances if it is an HD account and user recovers from phrase in future)
     * If it is a Legacy or imported witness key or similar account then it will be gone forever
     * Generally prefer 'deleteAccount' and use this with caution
     */
    Napi::Value purgeAccount(const Napi::CallbackInfo& info);

    /** List all currently visible accounts in the wallet */
    Napi::Value listAccounts(const Napi::CallbackInfo& info);

};
#endif //DJINNI_GENERATED_NJSIACCOUNTSCONTROLLER_HPP
