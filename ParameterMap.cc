#include "ParameterMap.h"

#include <third_party/WebKit/WebCore/bindings/v8/V8Binding.h>
#include <V8ImageData.h>


Chromix::ParameterMap::ParameterMap() : imageParamMap(), stringParamMap()
{
}

void Chromix::ParameterMap::registerStringParameter(const WTF::String& name, const WTF::String& description) {
    stringParamMap.set(name, Chromix::StringParameter(description));
}

void Chromix::ParameterMap::registerImageParameter(const WTF::String& name, const WTF::String& description) {
    imageParamMap.set(name, Chromix::ImageParameter(description));
}

v8::Handle<v8::Value> Chromix::ParameterMap::getImageParameterValue(const WTF::String& name) {
    if (imageParamMap.contains(name)) {
        ImageParameter param = imageParamMap.get(name);
        WebCore::ImageData *imageData = param.getValue();
        if (imageData)
            return WebCore::toV8(imageData);
    }
    return v8::Undefined();
}

v8::Handle<v8::Value> Chromix::ParameterMap::getStringParameterValue(const WTF::String& name) {
    if (stringParamMap.contains(name)) {
        StringParameter param = stringParamMap.get(name);
        return WebCore::v8String(param.getValue());
    }
    return v8::Undefined();
}

// http://webkit.org/coding/RefPtr.html
unsigned char* Chromix::ParameterMap::writeableDataForImageParameter(const WTF::String& name, unsigned int width, unsigned int height) {
    // First check if this is a valid image param
    if (!imageParamMap.contains(name))
        return NULL;

    ImageParameter param = imageParamMap.get(name);
    WebCore::ImageData *imageData = param.getValue();

    // Param has no ImageData yet, or if it has one of the wrong size,
    // create and set or replace.
    if (imageData == NULL || (width != imageData->width() || height != imageData->height())) {
        WTF::RefPtr<WebCore::ImageData> rpImageData = WebCore::ImageData::create(width, height);
        param.setValue(rpImageData);
        imageParamMap.set(name, param);
        imageData = rpImageData.get();
    }

    // Return the raw data from the CanvasPixelBuffer
    return imageData->data()->data()->data();
}
