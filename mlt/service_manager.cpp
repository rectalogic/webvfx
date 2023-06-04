// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <iterator>
#include <signal.h>
#include <sstream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vfxpipe.h>

extern "C" {
#include <framework/mlt_factory.h>
#include <framework/mlt_frame.h>
#include <framework/mlt_image.h>
#include <framework/mlt_log.h>
#include <framework/mlt_producer.h>
}
#include "service_manager.h"

namespace WebVfxPlugin {

class ImageProducer {
public:
    ImageProducer(mlt_producer producer)
        : producerFrame(0)
        , producer(producer)
    {
    }

    ~ImageProducer()
    {
        if (producerFrame)
            mlt_frame_close(producerFrame);
        mlt_producer_close(producer);
    }

    bool isPositionValid(mlt_position position)
    {
        return position < mlt_producer_get_playtime(producer);
    }

    mlt_image_s produceImage(mlt_position position, int width, int height)
    {
        // Close previous frame and request a new one.
        // We don't close the current frame because the image data we return
        // needs to remain valid until we are rendered.
        if (producerFrame) {
            mlt_frame_close(producerFrame);
            producerFrame = 0;
        }
        mlt_producer_seek(producer, position);
        mlt_service_get_frame(MLT_PRODUCER_SERVICE(producer), &producerFrame, 0);

        mlt_image_format format = mlt_image_rgba;
        uint8_t* image = NULL;
        int error = mlt_frame_get_image(producerFrame, &image, &format,
            &width, &height, 0);
        if (error)
            return mlt_image_s();
        mlt_image_s producedImage;
        mlt_image_set_values(&producedImage, image, format, width, height);
        return producedImage;
    }

private:
    mlt_frame producerFrame;
    mlt_producer producer;
};

////////////////////////

ServiceManager::ServiceManager(mlt_service service)
    : service(service)
    , length(0)
    , pid(0)
    , pipeRead(-1)
    , pipeWrite(-1)
    , imageProducers(0)
{
    mlt_properties_set(MLT_SERVICE_PROPERTIES(service), "factory", mlt_environment("MLT_PRODUCER"));
}

ServiceManager::~ServiceManager()
{
    if (pipeRead != -1)
        close(pipeRead);
    if (pipeWrite != -1)
        close(pipeWrite);
    if (imageProducers) {
        for (std::vector<ImageProducer*>::iterator it = imageProducers->begin();
             it != imageProducers->end(); it++) {
            delete *it;
        }
        delete imageProducers;
    }
}

bool ServiceManager::initialize(mlt_position length)
{
    // Return if already initialized
    if (pid == -1 || pid > 0)
        return true;

    this->length = length;
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);

    // Create and initialize webvfx
    const char* command = mlt_properties_get(properties, "resource");
    if (!command) {
        pid = -1;
        mlt_log_error(service, "No 'resource' property found\n");
        return false;
    }

    auto spawnErrorHandler = [this](std::string msg) {
        mlt_log_error(service, "%s", msg.c_str());
    };
    pid = VfxPipe::spawnProcess(&pipeRead, &pipeWrite, command, spawnErrorHandler);
    if (pid == -1) {
        return false;
    }

    // Find extra image producers - "producer.N.resource"
    char* factory = mlt_properties_get(properties, "factory");
    for (int i = 0; true; i++) {
        std::ostringstream os("producer.", std::ostringstream::ate);
        os << i << ".resource";
        auto resourceName = os.str();
        char* resource = mlt_properties_get(properties, resourceName.c_str());
        if (!resource) {
            break;
        }
        if (!imageProducers)
            imageProducers = new std::vector<ImageProducer*>(2);
        mlt_producer producer = mlt_factory_producer(mlt_service_profile(service), factory, resource);
        if (!producer) {
            mlt_log_error(service, "%s: vfxpipe failed to create extra image producer for %s\n", __FUNCTION__, resourceName.c_str());
            return false;
        }
        // Copy producer.N.* properties onto producer
        os << i << ".";
        mlt_properties_pass(MLT_PRODUCER_PROPERTIES(producer), properties, os.str().c_str());
        // Append ImageProducer to vector
        imageProducers->insert(imageProducers->end(), new ImageProducer(producer));
    }

    return true;
}

int ServiceManager::render(VfxPipe::VideoFrame* vfxSourceImage, VfxPipe::VideoFrame* vfxTargetImage, mlt_image outputImage, mlt_position position)
{
    if (pipeRead == -1 || pipeWrite == -1)
        return 1;

    auto f = __FUNCTION__;
    auto ioErrorHandler = [this, f](int n, std::string msg = "") {
        if (n == -1) {
            mlt_log_error(service, "%s: Image IO failed: %s\n", f, msg.c_str());
        }
        close(pipeRead);
        pipeRead = -1;
        close(pipeWrite);
        pipeWrite = -1;
    };

    double time = (double)position / length;
    if (!VfxPipe::dataIO(pipeWrite, reinterpret_cast<const std::byte*>(&time), sizeof(time), write, ioErrorHandler)) {
        return 1;
    }

    // Output format
    VfxPipe::VideoFrame outputFrame(VfxPipe::VideoFrameFormat::RGBA32, outputImage->width, outputImage->height);
    if (!VfxPipe::writeVideoFrame(pipeWrite, &outputFrame, ioErrorHandler)) {
        return 1;
    }

    uint32_t frameCount = imageProducers->size();
    if (vfxSourceImage)
        frameCount++;
    if (vfxTargetImage)
        frameCount++;
    if (!VfxPipe::dataIO(pipeWrite, reinterpret_cast<const std::byte*>(&frameCount), sizeof(frameCount), write, ioErrorHandler)) {
        return 1;
    }

    if (vfxSourceImage) {
        if (!VfxPipe::writeVideoFrame(pipeWrite, vfxSourceImage, ioErrorHandler)) {
            return 1;
        }
    }
    if (vfxTargetImage) {
        if (!VfxPipe::writeVideoFrame(pipeWrite, vfxTargetImage, ioErrorHandler)) {
            return 1;
        }
    }

    // Produce any extra images
    if (imageProducers) {
        for (std::vector<ImageProducer*>::iterator it = imageProducers->begin();
             it != imageProducers->end(); it++) {
            ImageProducer* imageProducer = *it;
            if (imageProducer && imageProducer->isPositionValid(position)) {
                mlt_image_s extraImage = imageProducer->produceImage(position,
                    outputImage->width,
                    outputImage->height);
                if (!extraImage.data) {
                    mlt_log_error(service, "%s: vfxpipe failed to produce image for extra producer %ld\n", __FUNCTION__, it - imageProducers->begin());
                    return 1;
                }
                VfxPipe::VideoFrame vfxFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, outputImage->width, outputImage->height, reinterpret_cast<std::byte*>(extraImage.data));
                if (!VfxPipe::writeVideoFrame(pipeWrite, &vfxFrame, ioErrorHandler)) {
                    return 1;
                }
            }
        }
    }

    if (!VfxPipe::dataIO(pipeRead, reinterpret_cast<std::byte*>(outputImage->data), mlt_image_calculate_size(outputImage), read, ioErrorHandler)) {
        return 1;
    }
    return 0;
}

}