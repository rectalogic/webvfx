// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/image_map.h"

#include <third_party/WebKit/WebCore/config.h>
#include <v8/include/v8.h>
#include <third_party/WebKit/WebCore/bindings/v8/V8Binding.h>
#include <V8ImageData.h>

Chromix::ImageMap::ImageMap() : imageDataMap()
{
}

v8::Handle<v8::Value> Chromix::ImageMap::getImageParameterValue(const std::string& name) const {
    ImageDataMap::const_iterator it = imageDataMap.find(name);
    if (it != imageDataMap.end())
        return WebCore::toV8(it->second.get());
    return v8::Undefined();
}

// http://webkit.org/coding/RefPtr.html
unsigned char* Chromix::ImageMap::writeableDataForImageParameter(const std::string& name, unsigned int width, unsigned int height) {
    // This will create the RefPtr if one doesn't exist.
    WTF::RefPtr<WebCore::ImageData> rpImageData = imageDataMap[name];
    WebCore::ImageData *imageData = rpImageData.get();

    // Param has no ImageData yet, or if it has one of the wrong size,
    // create and set or replace.
    if (imageData == NULL || (width != imageData->width() || height != imageData->height())) {
        rpImageData = WebCore::ImageData::create(width, height);
        imageDataMap[name] = rpImageData;
        imageData = rpImageData.get();
    }

    // Return the raw data from the CanvasPixelBuffer
    return imageData->data()->data()->data();
}
