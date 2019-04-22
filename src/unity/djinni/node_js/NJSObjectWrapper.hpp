// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni
#ifndef DJINNI_GENERATED_NJSOBJECTWRAPPER_HPP
#define DJINNI_GENERATED_NJSOBJECTWRAPPER_HPP

#include <memory>

#include <nan.h>
#include <node.h>

namespace djinni
{
namespace js
{

template<typename T>
class ObjectWrapper: public Nan::ObjectWrap
{
public:
    static void Wrap(const std::shared_ptr<T>& realObjectPtr, v8::Local<v8::Object> newHandleExposedToJS)
    {
        auto wrapper = new ObjectWrapper(realObjectPtr);
        wrapper->Nan::ObjectWrap::Wrap(newHandleExposedToJS);
    }
    static std::shared_ptr<T> Unwrap(v8::Local<v8::Object> handle)
    {
        return Nan::ObjectWrap::Unwrap<ObjectWrapper<T>>(handle)->_ptr;
    }
private:
    explicit ObjectWrapper(const std::shared_ptr<T>& realObjectPtr)	: _ptr(realObjectPtr) {};
    std::shared_ptr<T> _ptr;
};
} //js
} // djinni
#endif //DJINNI_GENERATED_NJSOBJECTWRAPPER_HPP
