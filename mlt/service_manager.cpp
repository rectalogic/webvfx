// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <webvfx/webvfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_frame.h>
    #include <mlt/framework/mlt_producer.h>
}
#include "service_manager.h"


namespace MLTWebVFX
{
const char* ServiceManager::kURLPropertyName = "WebVFXURL";

////////////////////////

class ServiceParameters : public WebVFX::Parameters
{
public:
    ServiceParameters(mlt_service service)
        : properties(MLT_SERVICE_PROPERTIES(service)) {
    }

    double getNumberParameter(const std::string& name) {
        return mlt_properties_get_double(properties, name.c_str());
    }

    std::string getStringParameter(const std::string& name) {
        return mlt_properties_get(properties, name.c_str());
    }

private:
    mlt_properties properties;
};

////////////////////////

class ImageProducer
{
public:
    ImageProducer(const std::string& name, mlt_producer producer)
        : name(name)
        , producer(producer) {}

    ~ImageProducer() {
        mlt_producer_close(producer);
    }

    const std::string& getName() { return name; }

    int produceImage(mlt_position position, WebVFX::Image& targetImage) {
        if (position > mlt_producer_get_out(producer))
            return 0;
        mlt_producer_seek(producer, position);
        mlt_frame frame = NULL;
        int error = mlt_service_get_frame(MLT_PRODUCER_SERVICE(producer), &frame, 0);
        if (error)
            return error;
        mlt_frame_set_position(frame, position);

        mlt_image_format format = mlt_image_rgb24;
        uint8_t *image = NULL;
        int width = targetImage.width();
        int height = targetImage.height();
        error = mlt_frame_get_image(frame, &image, &format, &width, &height, 0);
        if (!error)
            targetImage.copyPixelsFrom(WebVFX::Image(image, width, height, width * height * WebVFX::Image::BytesPerPixel));
        mlt_frame_close(frame);
        return error;
    }

private:
    std::string name;
    mlt_producer producer;
};

////////////////////////

ServiceManager::ServiceManager(mlt_service service)
    : service(service)
    , effects(0)
    , imageProducers(0)
{
    mlt_properties_set(MLT_SERVICE_PROPERTIES(service), "factory", mlt_environment("MLT_PRODUCER"));
}

ServiceManager::~ServiceManager()
{
    if (effects)
        effects->destroy();
    if (imageProducers) {
        for (std::vector<ImageProducer*>::iterator it = imageProducers->begin();
             it != imageProducers->end(); it++) {
            delete *it;
        }
        delete imageProducers;
    }
}

bool ServiceManager::initialize(int width, int height)
{
    if (effects)
        return true;

    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);

    // Create and initialize Effects
    const char* url = mlt_properties_get(properties, ServiceManager::kURLPropertyName);
    if (!url) {
        mlt_log(service, MLT_LOG_ERROR, "No %s property found\n", ServiceManager::kURLPropertyName);
        return false;
    }
    effects = WebVFX::createEffects();
    bool result = effects->initialize(url, width, height, new ServiceParameters(service));
    if (!result) {
        effects->destroy(); effects = 0;
        mlt_log(service, MLT_LOG_ERROR, "Failed to create WebVFX Effects\n");
        return result;
    }

    // Iterate over image map - save source and target image names,
    // and create an ImageProducer for each extra image.
    char* factory = mlt_properties_get(properties, "factory");
    const WebVFX::Effects::ImageTypeMap& imageMap = effects->getImageTypeMap();
    for (WebVFX::Effects::ImageTypeMap::const_iterator it = imageMap.begin();
         it != imageMap.end(); it++) {

        const std::string& imageName = it->first;

        switch (it->second) {

        case WebVFX::Effects::SourceImageType:
            sourceImageName = imageName;
            break;

        case WebVFX::Effects::TargetImageType:
            targetImageName = imageName;
            break;

        case WebVFX::Effects::ExtraImageType:
        {
            if (!imageProducers)
                imageProducers = new std::vector<ImageProducer*>(3);

            // Property prefix "producer.<name>."
            std::string producerPrefix("producer.");
            producerPrefix.append(imageName).append(".");

            // Find producer.<name>.resource property
            std::string resourceName(producerPrefix);
            resourceName.append("resource");
            char* resource = mlt_properties_get(properties, resourceName.c_str());
            if (resource) {
                mlt_producer producer = mlt_factory_producer(mlt_service_profile(service), factory, resource);
                if (!producer) {
                    mlt_log(service, MLT_LOG_ERROR, "WebVFX failed to create extra image producer for %s\n", resourceName.c_str());
                    return false;
                }
                // Copy producer.<name>.* properties onto producer
                mlt_properties_pass(MLT_PRODUCER_PROPERTIES(producer), properties, producerPrefix.c_str());
                // Append ImageProducer to vector
                imageProducers->insert(imageProducers->end(), new ImageProducer(imageName, producer));
            }
            else
                mlt_log(service, MLT_LOG_WARNING, "WebVFX no producer resource property specified for extra image %s\n", resourceName.c_str());
            break;
        }

        default:
            mlt_log(service, MLT_LOG_ERROR, "Invalid WebVFX image type %d\n", it->second);
            break;
        }
    }

    return true;
}

void ServiceManager::copyImageForName(const std::string& name, const WebVFX::Image& fromImage)
{
    if (!name.empty()) {
        WebVFX::Image toImage = effects->getImage(name, fromImage.width(), fromImage.height());
        toImage.copyPixelsFrom(fromImage);
    }
}

int ServiceManager::render(WebVFX::Image& outputImage, mlt_position position)
{
    int error = 0;

    // Compute time
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);
    mlt_position in = mlt_properties_get_position(properties, "in");
    mlt_position length = mlt_properties_get_position(properties, "out") - in + 1;
    double time = (double)(position - in) / (double)length;

    // Produce any extra images
    if (imageProducers) {
        for (std::vector<ImageProducer*>::iterator it = imageProducers->begin();
             it != imageProducers->end(); it++) {
            ImageProducer* imageProducer = *it;
            if (imageProducer) {
                WebVFX::Image extraImage = effects->getImage(imageProducer->getName(), outputImage.width(), outputImage.height());
                error = imageProducer->produceImage(position, extraImage);
                if (error) {
                    mlt_log(service, MLT_LOG_ERROR, "WebVFX failed to produce image for name %s\n", imageProducer->getName().c_str());
                    return error;
                }
            }
        }
    }

    const WebVFX::Image renderedImage = effects->render(time, outputImage.width(), outputImage.height());
    renderedImage.copyPixelsTo(outputImage);

    return error;
}

}
