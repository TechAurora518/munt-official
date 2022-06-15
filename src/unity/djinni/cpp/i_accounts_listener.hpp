// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <string>

struct AccountRecord;

/** Interface to receive updates about accounts */
#ifdef DJINNI_NODEJS
#include "NJSIAccountsListener.hpp" 
#define IAccountsListener NJSIAccountsListener
#else

class IAccountsListener {
public:
    virtual ~IAccountsListener() {}

    /** Notify that the active account has changed */
    virtual void onActiveAccountChanged(const std::string & accountUUID) = 0;

    /** Notify that the active account name has changed */
    virtual void onActiveAccountNameChanged(const std::string & newAccountName) = 0;

    /** Notify that an account name has changed */
    virtual void onAccountNameChanged(const std::string & accountUUID, const std::string & newAccountName) = 0;

    /** Notify that a new account has been added */
    virtual void onAccountAdded(const std::string & accountUUID, const std::string & accountName) = 0;

    /** Notify that an account has been deleted */
    virtual void onAccountDeleted(const std::string & accountUUID) = 0;

    /** Notify that an account has been modified */
    virtual void onAccountModified(const std::string & accountUUID, const AccountRecord & accountData) = 0;
};
#endif
