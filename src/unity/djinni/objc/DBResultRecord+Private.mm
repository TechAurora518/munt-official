// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBResultRecord+Private.h"
#import "DJIMarshal+Private.h"
#include <cassert>

namespace djinni_generated {

auto ResultRecord::toCpp(ObjcType obj) -> CppType
{
    assert(obj);
    return {::djinni::Bool::toCpp(obj.result),
            ::djinni::String::toCpp(obj.info)};
}

auto ResultRecord::fromCpp(const CppType& cpp) -> ObjcType
{
    return [[DBResultRecord alloc] initWithResult:(::djinni::Bool::fromCpp(cpp.result))
                                             info:(::djinni::String::fromCpp(cpp.info))];
}

}  // namespace djinni_generated
