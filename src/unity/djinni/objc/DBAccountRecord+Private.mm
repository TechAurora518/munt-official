// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBAccountRecord+Private.h"
#import "DBAccountLinkRecord+Private.h"
#import "DJIMarshal+Private.h"
#include <cassert>

namespace djinni_generated {

auto AccountRecord::toCpp(ObjcType obj) -> CppType
{
    assert(obj);
    return {::djinni::String::toCpp(obj.UUID),
            ::djinni::String::toCpp(obj.label),
            ::djinni::String::toCpp(obj.state),
            ::djinni::String::toCpp(obj.type),
            ::djinni::Bool::toCpp(obj.isHD),
            ::djinni::List<::djinni_generated::AccountLinkRecord>::toCpp(obj.accountLinks)};
}

auto AccountRecord::fromCpp(const CppType& cpp) -> ObjcType
{
    return [[DBAccountRecord alloc] initWithUUID:(::djinni::String::fromCpp(cpp.UUID))
                                           label:(::djinni::String::fromCpp(cpp.label))
                                           state:(::djinni::String::fromCpp(cpp.state))
                                            type:(::djinni::String::fromCpp(cpp.type))
                                            isHD:(::djinni::Bool::fromCpp(cpp.isHD))
                                    accountLinks:(::djinni::List<::djinni_generated::AccountLinkRecord>::fromCpp(cpp.accountLinks))];
}

}  // namespace djinni_generated
