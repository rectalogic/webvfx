// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "service_manager.h"
extern "C" {
#include <framework/mlt_factory.h> // for mlt_environment, mlt_factory_producer
#include <framework/mlt_frame.h> // for mlt_frame_close, mlt_frame_get_image, mlt_frame_s
#include <framework/mlt_image.h> // for mlt_image_s, mlt_image_set_values
#include <framework/mlt_log.h> // for mlt_log_error
#include <framework/mlt_producer.h> // for mlt_producer_close, mlt_producer_get_playtime, mlt_producer_s, mlt_producer_seek, MLT_PRODUCER_PROPERTIES, MLT_PRODUCER_SERVICE
#include <framework/mlt_properties.h> // for mlt_properties_get, mlt_properties_pass, mlt_properties_set, mlt_properties_s
}
#include <sstream> // for basic_ostream::operator<<, operator<<, ostringstream, basic_ostream, basic_ostream<>::__ostream_type
#include <stdint.h> // for uint8_t
#include <string.h> // for NULL
#include <string> // for basic_string, string
#include <vector> // for vector, allocator, vector<>::iterator
#include <vfxpipe.h> // for SourceVideoFrame, FrameServer

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
    , frameServer(nullptr)
    , imageProducers(0)
{
    mlt_properties_set(MLT_SERVICE_PROPERTIES(service), "factory", mlt_environment("MLT_PRODUCER"));
}

ServiceManager::~ServiceManager()
{
    delete frameServer;
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
    if (frameServer)
        return true;

    this->length = length;
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);

    // Create and initialize webvfx
    const char* qmlUrl = mlt_properties_get(properties, "resource");
    if (!qmlUrl) {
        mlt_log_error(service, "No 'resource' property found\n");
        return false;
    }

    frameServer = new VfxPipe::FrameServer(qmlUrl);

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

int ServiceManager::render(VfxPipe::SourceVideoFrame* vfxSourceImage, VfxPipe::SourceVideoFrame* vfxTargetImage, mlt_image outputImage, mlt_position position)
{
    if (!frameServer)
        return 1;

    auto f = __FUNCTION__;
    auto errorHandler = [this, f](std::string msg) {
        if (!msg.empty()) {
            mlt_log_error(service, "%s: render failed: %s\n", f, msg.c_str());
        }
    };

    double time = (double)position / length;

    std::vector<VfxPipe::SourceVideoFrame> sourceFrames;
    if (vfxSourceImage)
        sourceFrames.push_back(*vfxSourceImage);
    if (vfxTargetImage)
        sourceFrames.push_back(*vfxTargetImage);

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
                    delete frameServer;
                    frameServer = nullptr;
                    return 1;
                }
                VfxPipe::SourceVideoFrame vfxFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, outputImage->width, outputImage->height, reinterpret_cast<const std::byte*>(extraImage.data));
                sourceFrames.push_back(vfxFrame);
            }
        }
    }

    VfxPipe::RenderedVideoFrame outputFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, outputImage->width, outputImage->height, reinterpret_cast<std::byte*>(outputImage->data));
    if (!frameServer->renderFrame(time, sourceFrames, outputFrame, errorHandler)) {
        delete frameServer;
        frameServer = nullptr;
        return 1;
    }
    return 0;
}

}