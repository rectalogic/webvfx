// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/ParameterMap.h"
#include <v8/include/v8.h>

Chromix::ParameterMap::ParameterMap() : imageParamMap(), paramMap()
{
}

void Chromix::ParameterMap::setParameterValue(const WTF::String& name, WTF::PassRefPtr<ParameterValue> prpParam) {
    paramMap.set(name, prpParam);
}

v8::Handle<v8::Value> Chromix::ParameterMap::getImageParameterValue(const WTF::String& name) const {
    if (imageParamMap.contains(name)) {
        ImageParameterValue param = imageParamMap.get(name);
        return param.getV8Value();
    }
    return v8::Undefined();
}

v8::Handle<v8::Value> Chromix::ParameterMap::getParameterValue(const WTF::String& name) const {
    if (paramMap.contains(name)) {
        return paramMap.get(name)->getV8Value();
    }
    return v8::Undefined();
}

// http://webkit.org/coding/RefPtr.html
unsigned char* Chromix::ParameterMap::writeableDataForImageParameter(const WTF::String& name, unsigned int width, unsigned int height) {
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
