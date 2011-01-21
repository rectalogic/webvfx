// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_frame.h>
}
#include "chromix_task.h"

class ChromixProducerTask : public ChromixTask {
public:

    static int producerGetImage(mlt_frame frame, uint8_t **buffer, mlt_image_format *format, int *width, int *height, int writable) {
        int error = 0;

        // Obtain properties of frame
        mlt_properties properties = MLT_FRAME_PROPERTIES(frame);

        // Obtain the producer for this frame
        mlt_producer producer = (mlt_producer)mlt_properties_get_data(properties, "producer_chromix", NULL);

        // Compute position
        mlt_position position = mlt_frame_get_position(frame);

		// Allocate the image
        *format = mlt_image_rgb24a;
        int size = *width * *height * 4;
		*buffer = (uint8_t*)mlt_pool_alloc(size);
        if (!*buffer)
            return 1;

        // Update the frame
        mlt_properties_set_data(properties, "image", *buffer, size, mlt_pool_release, NULL);
        mlt_properties_set_int(properties, "width", *width);
        mlt_properties_set_int(properties, "height", *height);

        { // Scope the ServiceLock
            mlt_service service = MLT_PRODUCER_SERVICE(producer);
            ServiceLock lock(service);

            ChromixProducerTask* task = (ChromixProducerTask*)getTask(service);
            ChromixRawImage targetImage(*buffer, *width, *height);
            error = task->renderToImageForPosition(targetImage, position);
        }

        return error;
    }

    ChromixProducerTask(mlt_producer producer, const std::string& serviceName)
        : ChromixTask(MLT_PRODUCER_SERVICE(producer), serviceName) {}

protected:
    int performTask() {
        return 0;
    };
};

static int chromix_producer_get_frame(mlt_producer producer, mlt_frame_ptr frame, int index) {
    // Generate a frame
    *frame = mlt_frame_init(MLT_PRODUCER_SERVICE(producer));

    if (*frame != NULL) {
        // Obtain properties of frame and producer
        mlt_properties properties = MLT_FRAME_PROPERTIES(*frame);

        // Obtain properties of producer
        mlt_properties producer_props = MLT_PRODUCER_PROPERTIES(producer);

        // Set the producer on the frame properties
        mlt_properties_set_data(properties, "producer_chromix", producer, 0, NULL, NULL);

        // Update timecode on the frame we're creating
        mlt_frame_set_position(*frame, mlt_producer_position(producer));

        // Set producer-specific frame properties
        mlt_properties_set_int(properties, "progressive", 1);
        mlt_properties_set_double(properties, "aspect_ratio", mlt_properties_get_double(producer_props, "aspect_ratio"));

        // Push the get_image method
        mlt_frame_push_get_image(*frame, ChromixProducerTask::producerGetImage);
    }

    // Calculate the next timecode
    mlt_producer_prepare_next(producer);

    return 0;
}

mlt_producer chromix_producer_create(const char* service_name) {
    mlt_producer self = mlt_producer_new();
    if (self) {
        self->get_frame = chromix_producer_get_frame;
        if (!new ChromixProducerTask(self, service_name)) {
            mlt_producer_close(self);
            self = NULL;
        }
        return self;
    }
    return NULL;
}
