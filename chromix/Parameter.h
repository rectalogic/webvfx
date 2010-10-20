#ifndef MOTIONBOX_CHROMIX_PARAMETER_H_
#define MOTIONBOX_CHROMIX_PARAMETER_H_

#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>


namespace Chromix {

class Parameter {
public:
    Parameter() {};
    Parameter(WTF::String const& description) : description(description) {}
    virtual ~Parameter() {};

    const WTF::String getDescription() const { return description; }

private:
    WTF::String description;
};


class ImageParameter : public Parameter {
public:
    ImageParameter() {};
    ImageParameter(WTF::String const& description) : Parameter(description) {};
    virtual ~ImageParameter() {};

    void setValue(WTF::PassRefPtr<WebCore::ImageData> value) { imageDataValue = value; }
    WebCore::ImageData* getValue() const { return imageDataValue.get(); }

private:
    WTF::RefPtr<WebCore::ImageData> imageDataValue;
};


class StringParameter : public Parameter {
public:
    StringParameter() {};
    StringParameter(WTF::String const& description) : Parameter(description) {};
    virtual ~StringParameter() {};

    void setValue(WTF::String const& value) { stringValue = value; }
    const WTF::String getValue() const { return stringValue; }

private:
    WTF::String stringValue;
};

}

#endif

