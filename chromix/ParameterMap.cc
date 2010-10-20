#include "chromix/ParameterMap.h"
#include <v8/include/v8.h>

Chromix::ParameterMap::ParameterMap() : imageParamMap(), paramMap()
{
}

void Chromix::ParameterMap::setParameterValue(WTF::String const& name, Chromix::ParameterValue const &param) {
    paramMap.set(name, param);
}

v8::Handle<v8::Value> Chromix::ParameterMap::getImageParameterValue(WTF::String const& name) const {
    if (imageParamMap.contains(name)) {
        ImageParameterValue param = imageParamMap.get(name);
        return param.getV8Value();
    }
    return v8::Undefined();
}

v8::Handle<v8::Value> Chromix::ParameterMap::getParameterValue(WTF::String const& name) const {
    if (paramMap.contains(name)) {
        ParameterValue param = paramMap.get(name);
        return param.getV8Value();
    }
    return v8::Undefined();
}

// http://webkit.org/coding/RefPtr.html
unsigned char* Chromix::ParameterMap::writeableDataForImageParameter(WTF::String const& name, unsigned int width, unsigned int height) {
    // This will create the param if one doesn't exist.
    ImageParameterValue param = imageParamMap.get(name);
    WebCore::ImageData *imageData = param.getValue();

    // Param has no ImageData yet, or if it has one of the wrong size,
    // create and set or replace.
    if (imageData == NULL || (width != imageData->width() || height != imageData->height())) {
        WTF::RefPtr<WebCore::ImageData> rpImageData = WebCore::ImageData::create(width, height);
        imageParamMap.set(name, ImageParameterValue(rpImageData));
        imageData = rpImageData.get();
    }

    // Return the raw data from the CanvasPixelBuffer
    return imageData->data()->data()->data();
}
