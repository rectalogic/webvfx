// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_frame.h>
}
#include "webfx_service.h"
#include "service_manager.h"

static const char* kWebFXProducerPropertyName = "WebFXProducer";

static int producerGetImage(mlt_frame frame, uint8_t **buffer, mlt_image_format *format, int *width, int *height, int writable) {
    int error = 0;

    // Obtain properties of frame
    mlt_properties properties = MLT_FRAME_PROPERTIES(frame);

    // Obtain the producer for this frame
    mlt_producer producer = (mlt_producer)mlt_properties_get_data(properties, kWebFXProducerPropertyName, NULL);

    // Compute position
    mlt_position position = mlt_frame_get_position(frame);

    // Allocate the image
    *format = mlt_image_rgb24;
    int size = *width * *height * 4;
    *buffer = (uint8_t*)mlt_pool_alloc(size);
    if (!*buffer)
        return 1;

    // Update the frame
    mlt_properties_set_data(properties, "image", *buffer, size, mlt_pool_release, NULL);
    mlt_properties_set_int(properties, "width", *width);
    mlt_properties_set_int(properties, "height", *height);

    { // Scope the lock
        MLTWebFX::ServiceManager manager(MLT_PRODUCER_SERVICE(producer));
        if (!manager.initialize(*width, *height))
            return 1;

        //XXX fix this
        ChromixProducerTask* task = (ChromixProducerTask*)getTask(service);
        ChromixRawImage targetImage(*buffer, *width, *height);
        error = task->renderToImageForPosition(targetImage, position);
    }

    return error;
}

static int getFrame(mlt_producer producer, mlt_frame_ptr frame, int index) {
    // Generate a frame
    *frame = mlt_frame_init(MLT_PRODUCER_SERVICE(producer));

    if (*frame) {
        // Obtain properties of frame and producer
        mlt_properties properties = MLT_FRAME_PROPERTIES(*frame);

        // Obtain properties of producer
        mlt_properties producer_props = MLT_PRODUCER_PROPERTIES(producer);

        // Set the producer on the frame properties
        mlt_properties_set_data(properties, kWebFXProducerPropertyName, producer, 0, NULL, NULL);

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

mlt_service MLTWebFX::createProducer(const char* serviceName) {
    mlt_producer self = mlt_producer_new();
    if (self) {
        self->get_frame = getFrame;
        return MLT_PRODUCER_SERVICE(self);
    }
    return 0;
}
