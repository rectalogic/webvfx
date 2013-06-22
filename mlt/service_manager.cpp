// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <webvfx/webvfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_frame.h>
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_consumer.h>
}
#include "service_manager.h"


namespace MLTWebVfx
{

////////////////////////

class ServiceParameters : public WebVfx::Parameters
{
public:
    ServiceParameters(mlt_service service)
        : properties(MLT_SERVICE_PROPERTIES(service)) {
    }

    double getNumberParameter(const QString& name) {
        return mlt_properties_get_double(properties, name.toLatin1().constData());
    }

    QString getStringParameter(const QString& name) {
        return QString::fromUtf8(mlt_properties_get(properties, name.toLatin1().constData()));
    }

private:
    mlt_properties properties;
};

////////////////////////

class ImageProducer
{
public:
    ImageProducer(const QString& name, mlt_producer producer)
        : name(name)
        , producerFrame(0)
        , producer(producer) {}

    ~ImageProducer() {
        if (producerFrame)
            mlt_frame_close(producerFrame);
        mlt_producer_close(producer);
    }

    const QString& getName() { return name; }

    bool isPositionValid(mlt_position position) {
        return position < mlt_producer_get_playtime(producer);
    }

    WebVfx::Image produceImage(mlt_position position, int width, int height) {
        // Close previous frame and request a new one.
        // We don't close the current frame because the image data we return
        // needs to remain valid until we are rendered.
        if (producerFrame) {
            mlt_frame_close(producerFrame);
            producerFrame = 0;
        }
        mlt_producer_seek(producer, position);
        mlt_service_get_frame(MLT_PRODUCER_SERVICE(producer), &producerFrame, 0);

        mlt_image_format format = mlt_image_rgb24;
        uint8_t *image = NULL;
        int error = mlt_frame_get_image(producerFrame, &image, &format,
                                        &width, &height, 0);
        if (error)
            return WebVfx::Image();
        return WebVfx::Image(image, width, height, width * height * 3);
    }

private:
    QString name;
    mlt_frame producerFrame;
    mlt_producer producer;
};

////////////////////////

ServiceManager::ServiceManager(mlt_service service)
    : service(service)
    , event(0)
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
    const char* fileName = mlt_properties_get(properties, "resource");
    if (!fileName) {
        mlt_log(service, MLT_LOG_ERROR, "No 'resource' property found\n");
        return false;
    }
    effects = WebVfx::createEffects(fileName, width, height,
                                    new ServiceParameters(service));
    if (!effects) {
        mlt_log(service, MLT_LOG_ERROR,
                "Failed to create WebVfx Effects for resource %s\n", fileName);
        return false;
    }

    // Iterate over image map - save source and target image names,
    // and create an ImageProducer for each extra image.
    char* factory = mlt_properties_get(properties, "factory");
    WebVfx::Effects::ImageTypeMapIterator it(effects->getImageTypeMap());
    while (it.hasNext()) {
        it.next();

        const QString& imageName = it.key();

        switch (it.value()) {

        case WebVfx::Effects::SourceImageType:
            sourceImageName = imageName;
            break;

        case WebVfx::Effects::TargetImageType:
            targetImageName = imageName;
            break;

        case WebVfx::Effects::ExtraImageType:
        {
            if (!imageProducers)
                imageProducers = new std::vector<ImageProducer*>(3);

            // Property prefix "producer.<name>."
            QString producerPrefix("producer.");
            producerPrefix.append(imageName).append(".");

            // Find producer.<name>.resource property
            QString resourceName(producerPrefix);
            resourceName.append("resource");
            char* resource = mlt_properties_get(properties, resourceName.toLatin1().constData());
            if (resource) {
                mlt_producer producer = mlt_factory_producer(mlt_service_profile(service), factory, resource);
                if (!producer) {
                    mlt_log(service, MLT_LOG_ERROR, "WebVfx failed to create extra image producer for %s\n", resourceName.toLatin1().constData());
                    return false;
                }
                // Copy producer.<name>.* properties onto producer
                mlt_properties_pass(MLT_PRODUCER_PROPERTIES(producer), properties, producerPrefix.toLatin1().constData());
                // Append ImageProducer to vector
                imageProducers->insert(imageProducers->end(), new ImageProducer(imageName, producer));
            }
            else
                mlt_log(service, MLT_LOG_WARNING, "WebVfx no producer resource property specified for extra image %s\n", resourceName.toLatin1().constData());
            break;
        }

        default:
            mlt_log(service, MLT_LOG_ERROR, "Invalid WebVfx image type %d\n", it.value());
            break;
        }
    }

    return true;
}

void ServiceManager::setImageForName(const QString& name, WebVfx::Image* image)
{
    if (!name.isEmpty())
        effects->setImage(name, image);
}

static void consumerStoppingListener(mlt_properties owner, ServiceManager* self)
{
    Q_UNUSED(owner);
    self->onConsumerStopping();
}

int ServiceManager::render(WebVfx::Image* outputImage, mlt_position position, mlt_position length)
{
    double time = length > 0 ? position / (double)length : 0;

    // If there is a consumer property, listen to the consumer-stopping event to cancel rendering.
    if (!event && mlt_properties_get_data(MLT_SERVICE_PROPERTIES(service), "consumer", 0)) {
        mlt_consumer consumer = static_cast<mlt_consumer>(mlt_properties_get_data(MLT_SERVICE_PROPERTIES(service), "consumer", 0));
        event = mlt_events_listen(MLT_CONSUMER_PROPERTIES(consumer), this, "consumer-stopping",
            reinterpret_cast<mlt_listener>(MLTWebVfx::consumerStoppingListener));
    }

    // Produce any extra images
    if (imageProducers) {
        for (std::vector<ImageProducer*>::iterator it = imageProducers->begin();
             it != imageProducers->end(); it++) {
            ImageProducer* imageProducer = *it;
            if (imageProducer && imageProducer->isPositionValid(position)) {
                WebVfx::Image extraImage =
                    imageProducer->produceImage(position,
                                                outputImage->width(),
                                                outputImage->height());
                if (extraImage.isNull()) {
                    mlt_log(service, MLT_LOG_ERROR, "WebVfx failed to produce image for name %s\n", imageProducer->getName().toLatin1().constData());
                    return 1;
                }
                effects->setImage(imageProducer->getName(), &extraImage);
            }
        }
    }

    return !effects->render(time, outputImage);
}

void ServiceManager::onConsumerStopping()
{
    mlt_consumer consumer = static_cast<mlt_consumer>(mlt_properties_get_data(MLT_SERVICE_PROPERTIES(service), "consumer", 0));
    mlt_events_disconnect(MLT_CONSUMER_PROPERTIES(consumer), this);
    effects->renderComplete(false);
}

}
