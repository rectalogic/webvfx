#ifndef MOTIONBOX_CHROMIX_PARAMETERVALUE_H_
#define MOTIONBOX_CHROMIX_PARAMETERVALUE_H_

#include <third_party/WebKit/JavaScriptCore/wtf/RefCounted.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefPtr.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>

namespace v8 {
    template <class T> class Handle;
    class Value;
}

namespace Chromix {

class ParameterValue {
public:
    ParameterValue() {};
    virtual ~ParameterValue() {};
    virtual v8::Handle<v8::Value> getV8Value() const;
};


class ImageParameterValue : public ParameterValue {
public:
    ImageParameterValue() {};
    ImageParameterValue(WTF::PassRefPtr<WebCore::ImageData> value) : imageData(value) {};
    virtual ~ImageParameterValue() {};

    WebCore::ImageData* getValue() const { return imageData.get(); }
    virtual v8::Handle<v8::Value> getV8Value() const;

private:
    WTF::RefPtr<WebCore::ImageData> imageData;
};


template <typename T>
class PrimitiveParameterValue : public ParameterValue {
public:
    PrimitiveParameterValue(T const& value) : value(value) {};
    virtual ~PrimitiveParameterValue() {};
    const T getValue() const { return value; }
private:
    T value;
};


class StringParameterValue : public PrimitiveParameterValue<WTF::String> {
public:
    StringParameterValue(WTF::String const& value) : PrimitiveParameterValue<WTF::String>(value) {};
    virtual ~StringParameterValue() {};

    virtual v8::Handle<v8::Value> getV8Value() const;
};


class BooleanParameterValue : public PrimitiveParameterValue<bool> {
public:
    BooleanParameterValue(bool value) : PrimitiveParameterValue<bool>(value) {};
    virtual ~BooleanParameterValue() {};

    virtual v8::Handle<v8::Value> getV8Value() const;
};


class NumberParameterValue : public PrimitiveParameterValue<double> {
public:
    NumberParameterValue(double value) : PrimitiveParameterValue<double>(value) {};
    virtual ~NumberParameterValue() {};

    virtual v8::Handle<v8::Value> getV8Value() const;
};

}

#endif

