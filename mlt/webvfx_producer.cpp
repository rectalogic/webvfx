// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_frame.h>
}
#include <webvfx/image.h>
#include "service_locker.h"
#include "service_manager.h"
#include "webvfx_service.h"

static const char* kWebVFXProducerPropertyName = "WebVFXProducer";

static int producerGetImage(mlt_frame frame, uint8_t **buffer, mlt_image_format *format, int *width, int *height, int /*writable*/) {
    int error = 0;

    // Obtain properties of frame
    mlt_properties properties = MLT_FRAME_PROPERTIES(frame);

    // Obtain the producer for this frame
    mlt_producer producer = (mlt_producer)mlt_properties_get_data(properties, kWebVFXProducerPropertyName, NULL);

    // Compute position
    mlt_position position = mlt_frame_get_position(frame);

    // Allocate the image
    *format = mlt_image_rgb24;
    int size = *width * *height * WebVFX::Image::BytesPerPixel;
    *buffer = (uint8_t*)mlt_pool_alloc(size);
    if (!*buffer)
        return 1;

    // Update the frame
    mlt_properties_set_data(properties, "image", *buffer, size, mlt_pool_release, NULL);
    mlt_properties_set_int(properties, "width", *width);
    mlt_properties_set_int(properties, "height", *height);

    { // Scope the lock
        MLTWebVFX::ServiceLocker locker(MLT_PRODUCER_SERVICE(producer));
        if (!locker.initialize(*width, *height))
            return 1;

        WebVFX::Image outputImage(*buffer, *width, *height, size);
        locker.getManager()->render(outputImage, position);
    }

    return error;
}

static int getFrame(mlt_producer producer, mlt_frame_ptr frame, int /*index*/) {
    // Generate a frame
    *frame = mlt_frame_init(MLT_PRODUCER_SERVICE(producer));

    if (*frame) {
        // Obtain properties of frame and producer
        mlt_properties properties = MLT_FRAME_PROPERTIES(*frame);

        // Obtain properties of producer
        mlt_properties producer_props = MLT_PRODUCER_PROPERTIES(producer);

        // Set the producer on the frame properties
        mlt_properties_set_data(properties, kWebVFXProducerPropertyName, producer, 0, NULL, NULL);

        // Update timecode on the frame we're creating
        mlt_frame_set_position(*frame, mlt_producer_position(producer));

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

mlt_service MLTWebVFX::createProducer() {
    mlt_producer self = mlt_producer_new();
    if (self) {
        self->get_frame = getFrame;
        return MLT_PRODUCER_SERVICE(self);
    }
    return 0;
}
