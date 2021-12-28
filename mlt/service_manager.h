// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBVFX_SERVICE_MANAGER_H_
#define MLTWEBVFX_SERVICE_MANAGER_H_

#include <QString>
extern "C" {
    #include <framework/mlt_service.h>
}
#include <vector>


namespace WebVfx
{
    class Effects;
    class Image;
}

namespace MLTWebVfx
{
class ImageProducer;
class ServiceLocker;

class ServiceManager
{
public:
    const QString& getSourceImageName() { return sourceImageName; }
    const QString& getTargetImageName() { return targetImageName; }
    void setImageForName(const QString& name, WebVfx::Image* image);
    int render(WebVfx::Image* outputImage, mlt_position position, mlt_position length);

private:
    friend class ServiceLocker;
    ServiceManager(mlt_service service);
    ~ServiceManager();
    bool initialize(int width, int height);

    mlt_service service;
    WebVfx::Effects* effects;

    QString sourceImageName;
    QString targetImageName;
    std::vector<ImageProducer*>* imageProducers;
};

}

#endif
