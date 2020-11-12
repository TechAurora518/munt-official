// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBIAccountsController+Private.h"
#import "DBIAccountsController.h"
#import "DBAccountRecord+Private.h"
#import "DBBalanceRecord+Private.h"
#import "DBIAccountsListener+Private.h"
#import "DBMutationRecord+Private.h"
#import "DBTransactionRecord+Private.h"
#import "DJICppWrapperCache+Private.h"
#import "DJIError.h"
#import "DJIMarshal+Private.h"
#include <exception>
#include <stdexcept>
#include <utility>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

@interface DBIAccountsController ()

- (id)initWithCpp:(const std::shared_ptr<::IAccountsController>&)cppRef;

@end

@implementation DBIAccountsController {
    ::djinni::CppProxyCache::Handle<std::shared_ptr<::IAccountsController>> _cppRefHandle;
}

- (id)initWithCpp:(const std::shared_ptr<::IAccountsController>&)cppRef
{
    if (self = [super init]) {
        _cppRefHandle.assign(cppRef);
    }
    return self;
}

+ (void)setListener:(nullable id<DBIAccountsListener>)accountslistener {
    try {
        ::IAccountsController::setListener(::djinni_generated::IAccountsListener::toCpp(accountslistener));
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSArray<DBAccountRecord *> *)listAccounts {
    try {
        auto objcpp_result_ = ::IAccountsController::listAccounts();
        return ::djinni::List<::djinni_generated::AccountRecord>::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (BOOL)setActiveAccount:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::setActiveAccount(::djinni::String::toCpp(accountUUID));
        return ::djinni::Bool::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)getActiveAccount {
    try {
        auto objcpp_result_ = ::IAccountsController::getActiveAccount();
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)createAccount:(nonnull NSString *)accountName
                        accountType:(nonnull NSString *)accountType {
    try {
        auto objcpp_result_ = ::IAccountsController::createAccount(::djinni::String::toCpp(accountName),
                                                                   ::djinni::String::toCpp(accountType));
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)getAccountName:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::getAccountName(::djinni::String::toCpp(accountUUID));
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (BOOL)renameAccount:(nonnull NSString *)accountUUID
       newAccountName:(nonnull NSString *)newAccountName {
    try {
        auto objcpp_result_ = ::IAccountsController::renameAccount(::djinni::String::toCpp(accountUUID),
                                                                   ::djinni::String::toCpp(newAccountName));
        return ::djinni::Bool::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (BOOL)deleteAccount:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::deleteAccount(::djinni::String::toCpp(accountUUID));
        return ::djinni::Bool::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (BOOL)purgeAccount:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::purgeAccount(::djinni::String::toCpp(accountUUID));
        return ::djinni::Bool::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)getAccountLinkURI:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::getAccountLinkURI(::djinni::String::toCpp(accountUUID));
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)getWitnessKeyURI:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::getWitnessKeyURI(::djinni::String::toCpp(accountUUID));
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)createAccountFromWitnessKeyURI:(nonnull NSString *)witnessKeyURI
                                      newAccountName:(nonnull NSString *)newAccountName {
    try {
        auto objcpp_result_ = ::IAccountsController::createAccountFromWitnessKeyURI(::djinni::String::toCpp(witnessKeyURI),
                                                                                    ::djinni::String::toCpp(newAccountName));
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSString *)getReceiveAddress:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::getReceiveAddress(::djinni::String::toCpp(accountUUID));
        return ::djinni::String::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSArray<DBTransactionRecord *> *)getTransactionHistory:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::getTransactionHistory(::djinni::String::toCpp(accountUUID));
        return ::djinni::List<::djinni_generated::TransactionRecord>::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSArray<DBMutationRecord *> *)getMutationHistory:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::getMutationHistory(::djinni::String::toCpp(accountUUID));
        return ::djinni::List<::djinni_generated::MutationRecord>::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull DBBalanceRecord *)getActiveAccountBalance {
    try {
        auto objcpp_result_ = ::IAccountsController::getActiveAccountBalance();
        return ::djinni_generated::BalanceRecord::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull DBBalanceRecord *)getAccountBalance:(nonnull NSString *)accountUUID {
    try {
        auto objcpp_result_ = ::IAccountsController::getAccountBalance(::djinni::String::toCpp(accountUUID));
        return ::djinni_generated::BalanceRecord::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

+ (nonnull NSDictionary<NSString *, DBBalanceRecord *> *)getAllAccountBalances {
    try {
        auto objcpp_result_ = ::IAccountsController::getAllAccountBalances();
        return ::djinni::Map<::djinni::String, ::djinni_generated::BalanceRecord>::fromCpp(objcpp_result_);
    } DJINNI_TRANSLATE_EXCEPTIONS()
}

namespace djinni_generated {

auto IAccountsController::toCpp(ObjcType objc) -> CppType
{
    if (!objc) {
        return nullptr;
    }
    return objc->_cppRefHandle.get();
}

auto IAccountsController::fromCppOpt(const CppOptType& cpp) -> ObjcType
{
    if (!cpp) {
        return nil;
    }
    return ::djinni::get_cpp_proxy<DBIAccountsController>(cpp);
}

}  // namespace djinni_generated

@end
