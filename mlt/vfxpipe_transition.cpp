// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <framework/mlt_transition.h>
    #include <framework/mlt_frame.h>
    #include <framework/mlt_log.h>
}
#include <cstring>
#include "factory.h"
#include "service_locker.h"
#include "service_manager.h"


static int transitionGetImage(mlt_frame aFrame, uint8_t **image, mlt_image_format *format, int *width, int *height, int /*writable*/) {
    int error = 0;

    mlt_frame bFrame = mlt_frame_pop_frame(aFrame);
    mlt_transition transition = (mlt_transition)mlt_frame_pop_service(aFrame);

    mlt_position length = mlt_transition_get_length(transition);

    // Get the aFrame image, we will write our output to it
    *format = mlt_image_rgb;
    if ((error = mlt_frame_get_image(aFrame, image, format, width, height, 1)) != 0)
        return error;
    // Get the bFrame image, we won't write to it
    uint8_t *bImage = NULL;
    int bWidth = 0, bHeight = 0;
    if ((error = mlt_frame_get_image(bFrame, &bImage, format, &bWidth, &bHeight, 0)) != 0)
        return error;

    { // Scope the lock
        VFXPipe::ServiceLocker locker(MLT_TRANSITION_SERVICE(transition));
        if (!locker.initialize(*width, *height, length))
            return 1;

        VFXPipe::ServiceManager* manager = locker.getManager();
        mlt_image_s renderedImage;
        mlt_image_set_values(&renderedImage, *image, *format, *width, *height);
        mlt_image_s targetImage;
        mlt_image_set_values(&targetImage, bImage, *format, bWidth, bHeight);
        manager->render(&renderedImage, &targetImage, &renderedImage);
    }

    return error;
}

static mlt_frame transitionProcess(mlt_transition transition, mlt_frame aFrame, mlt_frame bFrame) {
    mlt_frame_push_service(aFrame, transition);
    mlt_frame_push_frame(aFrame, bFrame);
    mlt_frame_push_get_image(aFrame, transitionGetImage);
    return aFrame;
}

mlt_service VFXPipe::createTransition() {
    mlt_transition self = mlt_transition_new();
    if (self) {
        self->process = transitionProcess;
        // Video only transition
        mlt_properties_set_int(MLT_TRANSITION_PROPERTIES(self), "_transition_type", 1);
        return MLT_TRANSITION_SERVICE(self);
    }
    return 0;
}
