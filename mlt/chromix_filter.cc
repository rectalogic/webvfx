// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_filter.h>
    #include <mlt/framework/mlt_frame.h>
    #include <mlt/framework/mlt_log.h>
}
#include "chromix_task.h"
#include "chromix_service.h"


class ChromixFilterTask : public ChromixTask {
public:

    static int filterGetImage(mlt_frame frame, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable) {
        int error = 0;

        // Get the filter
        mlt_filter filter = (mlt_filter)mlt_frame_pop_service(frame);

        // Compute time
        char *name = mlt_properties_get(MLT_FILTER_PROPERTIES(filter), "_unique_id");
        mlt_position position = mlt_properties_get_position(MLT_FRAME_PROPERTIES(frame), name);

        // Get the image, we will write our output to it
        *format = mlt_image_rgb24a;
        if ((error = mlt_frame_get_image(frame, image, format, width, height, 1)) != 0)
            return error;

        { // Scope the ServiceLock
            mlt_service service = MLT_FILTER_SERVICE(filter);
            ServiceLock lock(service);

            ChromixFilterTask* task = (ChromixFilterTask*)getTask(service);
            ChromixRawImage targetImage(*image, *width, *height);
            task->aTrackImage.set(*image, *width, *height);
            error = task->renderToImageForPosition(targetImage, position);
            task->aTrackImage.set();
        }

        return error;
    }

    ChromixFilterTask(mlt_filter filter, const std::string& serviceName)
        : ChromixTask(MLT_FILTER_SERVICE(filter), serviceName) {}

    int initialize() {
        int result = ChromixTask::initialize();
        if (result == 0) {
            // Get a image name
            mlt_properties metadata = getMetadata();
            const char* name = mlt_properties_get(metadata, A_IMAGE_METADATA_PROP);
            if (!name) {
                mlt_log(getService(), MLT_LOG_FATAL, "failed to find " A_IMAGE_METADATA_PROP " specifications in metadata\n");
                return 1;
            }
            aImageName = name;
        }
        return result;
    }

protected:
    int performTask() {
        return setImageForName(aTrackImage, aImageName);
    };

private:
    std::string aImageName;
    ChromixRawImage aTrackImage;
};


static mlt_frame chromix_filter_process(mlt_filter filter, mlt_frame frame) {
    // Store position on frame
    char *name = mlt_properties_get(MLT_FILTER_PROPERTIES(filter), "_unique_id");
    mlt_properties_set_position(MLT_FRAME_PROPERTIES(frame), name, mlt_frame_get_position(frame));
    // Push the frame filter
    mlt_frame_push_service(frame, filter);
    mlt_frame_push_get_image(frame, ChromixFilterTask::filterGetImage);

    return frame;
}

mlt_filter chromix_filter_create(const char* service_name) {
    mlt_filter self = mlt_filter_new();
    if (self) {
        self->process = chromix_filter_process;
        if (!new ChromixFilterTask(self, std::string(service_name))) {
            mlt_filter_close(self);
            self = NULL;
        }
        return self;
    }
    return NULL;
}
