// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "gulden_monitor_listener.hpp"
#include <memory>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

@protocol DBGuldenMonitorListener;

namespace djinni_generated {

class GuldenMonitorListener
{
public:
    using CppType = std::shared_ptr<::GuldenMonitorListener>;
    using CppOptType = std::shared_ptr<::GuldenMonitorListener>;
    using ObjcType = id<DBGuldenMonitorListener>;

    using Boxed = GuldenMonitorListener;

    static CppType toCpp(ObjcType objc);
    static ObjcType fromCppOpt(const CppOptType& cpp);
    static ObjcType fromCpp(const CppType& cpp) { return fromCppOpt(cpp); }

private:
    class ObjcProxy;
};

}  // namespace djinni_generated

