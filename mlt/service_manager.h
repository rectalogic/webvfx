// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBVFX_SERVICE_MANAGER_H_
#define MLTWEBVFX_SERVICE_MANAGER_H_

extern "C" {
    #include <mlt/framework/mlt_service.h>
}
#include <string>
#include <vector>

namespace WebVFX
{
    class WebEffects;
    class WebImage;
}

namespace MLTWebVFX
{
class ImageProducer;
class ServiceLocker;

class ServiceManager
{
public:
    const std::string& getSourceImageName() { return sourceImageName; }
    const std::string& getTargetImageName() { return targetImageName; }
    void copyImageForName(const std::string& name, const WebVFX::WebImage& fromImage);
    int render(WebVFX::WebImage& outputImage, mlt_position position);

    static const char* kURLPropertyName;

private:
    friend class ServiceLocker;
    ServiceManager(mlt_service service);
    ~ServiceManager();
    bool initialize(int width, int height);

    mlt_service service;
    WebVFX::WebEffects* webEffects;

    std::string sourceImageName;
    std::string targetImageName;
    std::vector<ImageProducer*>* imageProducers;
};

}

#endif
