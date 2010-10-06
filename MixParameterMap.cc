#include "MixParameterMap.h"

Chromix::MixParameterMap::MixParameterMap() : imageMap()
{
}


// http://webkit.org/coding/RefPtr.html
WTF::PassRefPtr<WebCore::ImageData> Chromix::MixParameterMap::imageDataForKey(WTF::String key, unsigned int width, unsigned int height) {
    WTF::RefPtr<WebCore::ImageData> imageData;
    // Found image for key, recreate if wrong size
    if (imageMap.contains(key)) {
        imageData = imageMap.get(key);
        if (width != imageData->width() || height != imageData->height()) {
            imageData = WebCore::ImageData::create(width, height);
            imageMap.set(key, imageData);
        }
    }
    // No image in map, create a new one
    else {
        imageData = WebCore::ImageData::create(width, height);
        imageMap.set(key, imageData);
    }
    return imageData.release(); //XXX figure out refcounting
}

