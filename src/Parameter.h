#ifndef MOTIONBOX_CHROMIX_PARAMETER_H_
#define MOTIONBOX_CHROMIX_PARAMETER_H_

#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>


namespace Chromix {

class Parameter {
public:
    Parameter() {};
    Parameter(const WTF::String& description);
    virtual ~Parameter() {};

    WTF::String getDescription() { return description; }

private:
    WTF::String description;
};


class ImageParameter : public Parameter {
public:
    ImageParameter() {};
    ImageParameter(const WTF::String& description) : Parameter(description) {};

    virtual ~ImageParameter() {};
    void setValue(WTF::PassRefPtr<WebCore::ImageData> value) { imageDataValue = value; }
    WebCore::ImageData* getValue() { return imageDataValue.get(); }

private:
    WTF::RefPtr<WebCore::ImageData> imageDataValue;
};


class StringParameter : public Parameter {
public:
    StringParameter() {};
    StringParameter(const WTF::String& description) : Parameter(description) {};

    virtual ~StringParameter() {};
    void setValue(WTF::String& value) { stringValue = value; }
    WTF::String getValue() { return stringValue; }

private:
    WTF::String stringValue;
};

}

#endif

