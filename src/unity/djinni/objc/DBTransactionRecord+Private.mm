// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBTransactionRecord+Private.h"
#import "DBOutputRecord+Private.h"
#import "DJIMarshal+Private.h"
#include <cassert>

namespace djinni_generated {

auto TransactionRecord::toCpp(ObjcType obj) -> CppType
{
    assert(obj);
    return {::djinni::String::toCpp(obj.txHash),
            ::djinni::I64::toCpp(obj.timestamp),
            ::djinni::I64::toCpp(obj.amount),
            ::djinni::I64::toCpp(obj.fee),
            ::djinni::List<::djinni_generated::OutputRecord>::toCpp(obj.receivedOutputs),
            ::djinni::List<::djinni_generated::OutputRecord>::toCpp(obj.sentOutputs)};
}

auto TransactionRecord::fromCpp(const CppType& cpp) -> ObjcType
{
    return [[DBTransactionRecord alloc] initWithTxHash:(::djinni::String::fromCpp(cpp.txHash))
                                             timestamp:(::djinni::I64::fromCpp(cpp.timestamp))
                                                amount:(::djinni::I64::fromCpp(cpp.amount))
                                                   fee:(::djinni::I64::fromCpp(cpp.fee))
                                       receivedOutputs:(::djinni::List<::djinni_generated::OutputRecord>::fromCpp(cpp.receivedOutputs))
                                           sentOutputs:(::djinni::List<::djinni_generated::OutputRecord>::fromCpp(cpp.sentOutputs))];
}

}  // namespace djinni_generated
