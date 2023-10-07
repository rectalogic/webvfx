// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
#include <framework/mlt_frame.h> // for mlt_frame_init, mlt_frame_push_get_image, mlt_frame_set_image, mlt_frame_set_position, MLT_FRAME_PROPERTIES, mlt_frame_s
#include <framework/mlt_image.h> // for mlt_image_set_values, mlt_image_s
#include <framework/mlt_pool.h> // for mlt_pool_alloc, mlt_pool_release
#include <framework/mlt_producer.h> // for mlt_producer_s, MLT_PRODUCER_SERVICE, mlt_producer_get_length, mlt_producer_new, mlt_producer_position, mlt_producer_prepare_next, MLT_PRODUCER_PROPERTIES
#include <framework/mlt_profile.h> // for mlt_profile_s
#include <framework/mlt_properties.h> // for mlt_properties_set_int, mlt_properties_get_data, mlt_properties_get_double, mlt_properties_get_position, mlt_properties_s, mlt_properties_set_data, mlt_properties_set_double, mlt_properties_set_position
#include <framework/mlt_service.h> // for mlt_service_s
#include <framework/mlt_types.h> // for mlt_producer, mlt_properties, mlt_frame, mlt_frame_ptr, mlt_image_format, mlt_image_rgba, mlt_position, mlt_profile, mlt_service
}
#include "factory.h" // for createProducer
#include "service_locker.h" // for ServiceLocker
#include "service_manager.h" // for ServiceManager
#include <cstddef> // for NULL
#include <stdint.h> // for uint8_t

static const char* kWebVfxProducerPropertyName = "WebVfxProducer";
static const char* kWebVfxPositionPropertyName = "webvfx.position";

static int producerGetImage(mlt_frame frame, uint8_t** buffer, mlt_image_format* format, int* width, int* height, int /*writable*/)
{
    int error = 0;

    // Obtain properties of frame
    mlt_properties properties = MLT_FRAME_PROPERTIES(frame);

    // Obtain the producer for this frame
    mlt_producer producer = (mlt_producer)mlt_properties_get_data(properties, kWebVfxProducerPropertyName, NULL);

    // Allocate the image
    *format = mlt_image_rgba;
    int size = *width * *height * 4;
    *buffer = (uint8_t*)mlt_pool_alloc(size);
    if (!*buffer)
        return 1;

    // Update the frame
    mlt_frame_set_image(frame, *buffer, size, mlt_pool_release);
    mlt_properties_set_int(properties, "width", *width);
    mlt_properties_set_int(properties, "height", *height);

    { // Scope the lock
        WebVfxPlugin::ServiceLocker locker(MLT_PRODUCER_SERVICE(producer));
        if (!locker.initialize(mlt_producer_get_length(producer)))
            return 1;

        mlt_image_s outputImage;
        mlt_image_set_values(&outputImage, *buffer, *format, *width, *height);
        locker.getManager()->render(nullptr, nullptr, &outputImage,
            mlt_properties_get_position(properties, kWebVfxPositionPropertyName));
    }

    return error;
}

static int getFrame(mlt_producer producer, mlt_frame_ptr frame, int /*index*/)
{
    // Generate a frame
    *frame = mlt_frame_init(MLT_PRODUCER_SERVICE(producer));

    if (*frame) {
        // Obtain properties of frame and producer
        mlt_properties properties = MLT_FRAME_PROPERTIES(*frame);

        // Obtain properties of producer
        mlt_properties producer_props = MLT_PRODUCER_PROPERTIES(producer);

        // Set the producer on the frame properties
        mlt_properties_set_data(properties, kWebVfxProducerPropertyName, producer, 0, NULL, NULL);

        // Update timecode on the frame we're creating
        mlt_position position = mlt_producer_position(producer);
        mlt_frame_set_position(*frame, position);
        mlt_properties_set_position(properties, kWebVfxPositionPropertyName, position);

        // Set producer-specific frame properties
        mlt_properties_set_int(properties, "progressive", 1);
        mlt_properties_set_double(properties, "aspect_ratio", mlt_properties_get_double(producer_props, "aspect_ratio"));

        // Push the get_image method
        mlt_frame_push_get_image(*frame, producerGetImage);
    }

    // Calculate the next timecode
    mlt_producer_prepare_next(producer);

    return 0;
}

mlt_service WebVfxPlugin::createProducer(mlt_profile profile)
{
    mlt_producer self = mlt_producer_new(profile);
    if (self) {
        self->get_frame = getFrame;
        return MLT_PRODUCER_SERVICE(self);
    }
    return 0;
}
