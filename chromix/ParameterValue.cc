#include "chromix/ParameterValue.h"

#include <third_party/WebKit/WebCore/config.h>
#include <v8/include/v8.h>
#include <third_party/WebKit/WebCore/bindings/v8/V8Binding.h>
#include <V8ImageData.h>


v8::Handle<v8::Value> Chromix::ParameterValue::getV8Value() const {
    return v8::Undefined();
};

v8::Handle<v8::Value> Chromix::ImageParameterValue::getV8Value() const {
    return WebCore::toV8(imageData.get());
}

v8::Handle<v8::Value> Chromix::StringParameterValue::getV8Value() const {
    return WebCore::v8String(getValue());
}

v8::Handle<v8::Value> Chromix::BooleanParameterValue::getV8Value() const {
    return WebCore::v8Boolean(getValue());
}

v8::Handle<v8::Value> Chromix::NumberParameterValue::getV8Value() const {
    return v8::Number::New(getValue());
}
