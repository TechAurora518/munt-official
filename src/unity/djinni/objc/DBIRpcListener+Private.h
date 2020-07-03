// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "i_rpc_listener.hpp"
#include <memory>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

@protocol DBIRpcListener;

namespace djinni_generated {

class IRpcListener
{
public:
    using CppType = std::shared_ptr<::IRpcListener>;
    using CppOptType = std::shared_ptr<::IRpcListener>;
    using ObjcType = id<DBIRpcListener>;

    using Boxed = IRpcListener;

    static CppType toCpp(ObjcType objc);
    static ObjcType fromCppOpt(const CppOptType& cpp);
    static ObjcType fromCpp(const CppType& cpp) { return fromCppOpt(cpp); }

private:
    class ObjcProxy;
};

}  // namespace djinni_generated

