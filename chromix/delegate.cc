#include "chromix/delegate.h"

#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/WebCore/bindings/v8/V8Binding.h>

v8::Handle<v8::Value> Chromix::Delegate::getUndefinedParameterValue() {
    return v8::Undefined();
}

v8::Handle<v8::Value> Chromix::Delegate::wrapParameterValue(bool value) {
    return WebCore::v8Boolean(value);
}

v8::Handle<v8::Value> Chromix::Delegate::wrapParameterValue(double value) {
    return v8::Number::New(value);
}

v8::Handle<v8::Value> Chromix::Delegate::wrapParameterValue(const std::string& value) {
    return WebCore::v8String(WTF::String::fromUTF8(value.data(), value.size()));
}
