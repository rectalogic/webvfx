// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/ParameterMap.h"
#include <v8/include/v8.h>

Chromix::ParameterMap::ParameterMap() : imageParamMap(), paramMap()
{
}

void Chromix::ParameterMap::setParameterValue(const std::string& name, WTF::PassRefPtr<ParameterValue> prpParam) {
    paramMap[name] = prpParam;
}

v8::Handle<v8::Value> Chromix::ParameterMap::getImageParameterValue(const std::string& name) const {
    ImageParamMap::const_iterator it = imageParamMap.find(name);
    if (it != imageParamMap.end())
        return it->second.getV8Value();
    return v8::Undefined();
}

v8::Handle<v8::Value> Chromix::ParameterMap::getParameterValue(const std::string& name) const {
    ParamMap::const_iterator it = paramMap.find(name);
    if (it != paramMap.end())
        return it->second->getV8Value();
    return v8::Undefined();
}

// http://webkit.org/coding/RefPtr.html
unsigned char* Chromix::ParameterMap::writeableDataForImageParameter(const std::string& name, unsigned int width, unsigned int height) {
    // This will create the param if one doesn't exist.
    ImageParameterValue param = imageParamMap[name];
    WebCore::ImageData *imageData = param.getValue();

    // Param has no ImageData yet, or if it has one of the wrong size,
    // create and set or replace.
    if (imageData == NULL || (width != imageData->width() || height != imageData->height())) {
        WTF::RefPtr<WebCore::ImageData> rpImageData = WebCore::ImageData::create(width, height);
        imageParamMap[name] = ImageParameterValue(rpImageData);
        imageData = rpImageData.get();
    }

    // Return the raw data from the CanvasPixelBuffer
    return imageData->data()->data()->data();
}
