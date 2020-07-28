// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBIWalletListener+Private.h"
#import "DBIWalletListener.h"
#import "DBBalanceRecord+Private.h"
#import "DBMutationRecord+Private.h"
#import "DBTransactionRecord+Private.h"
#import "DJIMarshal+Private.h"
#import "DJIObjcWrapperCache+Private.h"
#include <stdexcept>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

namespace djinni_generated {

class IWalletListener::ObjcProxy final
: public ::IWalletListener
, private ::djinni::ObjcProxyBase<ObjcType>
{
    friend class ::djinni_generated::IWalletListener;
public:
    using ObjcProxyBase::ObjcProxyBase;
    void notifyBalanceChange(const ::BalanceRecord & c_new_balance) override
    {
        @autoreleasepool {
            [djinni_private_get_proxied_objc_object() notifyBalanceChange:(::djinni_generated::BalanceRecord::fromCpp(c_new_balance))];
        }
    }
    void notifyNewMutation(const ::MutationRecord & c_mutation, bool c_self_committed) override
    {
        @autoreleasepool {
            [djinni_private_get_proxied_objc_object() notifyNewMutation:(::djinni_generated::MutationRecord::fromCpp(c_mutation))
                                                          selfCommitted:(::djinni::Bool::fromCpp(c_self_committed))];
        }
    }
    void notifyUpdatedTransaction(const ::TransactionRecord & c_transaction) override
    {
        @autoreleasepool {
            [djinni_private_get_proxied_objc_object() notifyUpdatedTransaction:(::djinni_generated::TransactionRecord::fromCpp(c_transaction))];
        }
    }
};

}  // namespace djinni_generated

namespace djinni_generated {

auto IWalletListener::toCpp(ObjcType objc) -> CppType
{
    if (!objc) {
        return nullptr;
    }
    return ::djinni::get_objc_proxy<ObjcProxy>(objc);
}

auto IWalletListener::fromCppOpt(const CppOptType& cpp) -> ObjcType
{
    if (!cpp) {
        return nil;
    }
    return dynamic_cast<ObjcProxy&>(*cpp).djinni_private_get_proxied_objc_object();
}

}  // namespace djinni_generated
