#ifndef MOTIONBOX_CHROMIX_PARAMETERVALUE_H_
#define MOTIONBOX_CHROMIX_PARAMETERVALUE_H_

#include <third_party/WebKit/JavaScriptCore/wtf/RefCounted.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>

namespace v8 {
    template <class T> class Handle;
    class Value;
}

namespace Chromix {

class ImageParameterValue {
public:
    ImageParameterValue() {};
    ImageParameterValue(WTF::PassRefPtr<WebCore::ImageData> value) : imageData(value) {};
    ~ImageParameterValue() {};

    WebCore::ImageData* getValue() const { return imageData.get(); }
    v8::Handle<v8::Value> getV8Value() const;

private:
    WTF::RefPtr<WebCore::ImageData> imageData;
};


class ParameterValue : public WTF::RefCounted<ParameterValue>  {
public:
    virtual ~ParameterValue() {};
    virtual v8::Handle<v8::Value> getV8Value() const = 0;
protected:
    ParameterValue() {};
};

template <typename T>
class PrimitiveParameterValue : public ParameterValue  {
public:
    PrimitiveParameterValue(T const& value) : value(value) {};
    virtual ~PrimitiveParameterValue() {};
    const T getValue() const { return value; }
    virtual v8::Handle<v8::Value> getV8Value() const = 0;
private:
    T value;
};


class StringParameterValue : public PrimitiveParameterValue<WTF::String> {
public:
    static WTF::PassRefPtr<StringParameterValue> create(WTF::String const& value) { return adoptRef(new StringParameterValue(value)); }
    virtual ~StringParameterValue() {};
    virtual v8::Handle<v8::Value> getV8Value() const;

private:
    StringParameterValue(WTF::String const& value) : PrimitiveParameterValue<WTF::String>(value) {};
};


class BooleanParameterValue : public PrimitiveParameterValue<bool> {
public:
    static WTF::PassRefPtr<BooleanParameterValue> create(bool value) { return adoptRef(new BooleanParameterValue(value)); }
    virtual ~BooleanParameterValue() {};
    virtual v8::Handle<v8::Value> getV8Value() const;

private:
    BooleanParameterValue(bool value) : PrimitiveParameterValue<bool>(value) {};
};


class NumberParameterValue : public PrimitiveParameterValue<double> {
public:
    static WTF::PassRefPtr<NumberParameterValue> create(double value) { return adoptRef(new NumberParameterValue(value)); }
    virtual ~NumberParameterValue() {};
    virtual v8::Handle<v8::Value> getV8Value() const;

private:
    NumberParameterValue(double value) : PrimitiveParameterValue<double>(value) {};
};

}

#endif

