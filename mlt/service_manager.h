// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFXPIPE_SERVICE_MANAGER_H_
#define VFXPIPE_SERVICE_MANAGER_H_

extern "C" {
    #include <framework/mlt_service.h>
    #include <framework/mlt_image.h>
}
#include <vector>
#include <unistd.h>

namespace VFXPipe
{
class ImageProducer;
class ServiceLocker;

class ServiceManager
{
public:
    int render(mlt_image sourceImage, mlt_image targetImage, mlt_image outputImage, mlt_position position);

private:
    friend class ServiceLocker;
    ServiceManager(mlt_service service);
    ~ServiceManager();
    bool initialize(int width, int height, mlt_position length);
    template <typename T> bool dataIO(int fd, void *data, size_t size, T ioFunc);

    mlt_service service;
    mlt_position length;
    pid_t pid;
    int pipeRead;
    int pipeWrite;
    std::vector<ImageProducer*>* imageProducers;
};

}

#endif
