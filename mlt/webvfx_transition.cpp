// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_transition.h>
    #include <mlt/framework/mlt_frame.h>
    #include <mlt/framework/mlt_log.h>
}
#include "webvfx_service.h"
#include "service_locker.h"


static int transitionGetImage(mlt_frame aFrame, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable) {
    int error = 0;

    mlt_frame bFrame = mlt_frame_pop_frame(aFrame);
    mlt_transition transition = (mlt_transition)mlt_frame_pop_service(aFrame);
    mlt_properties transition_props = MLT_TRANSITION_PROPERTIES(transition);
    mlt_properties a_props = MLT_FRAME_PROPERTIES(aFrame);
    mlt_properties b_props = MLT_FRAME_PROPERTIES(bFrame);

    // Set consumer_aspect_ratio for a and b frame
    if (mlt_properties_get_double(a_props, "aspect_ratio") == 0.0)
        mlt_properties_set_double(a_props, "aspect_ratio", mlt_properties_get_double(a_props, "consumer_aspect_ratio"));
    if (mlt_properties_get_double(b_props, "aspect_ratio") == 0.0)
        mlt_properties_set_double(b_props, "aspect_ratio", mlt_properties_get_double(a_props, "consumer_aspect_ratio"));
    mlt_properties_set_double(b_props, "consumer_aspect_ratio", mlt_properties_get_double(a_props, "consumer_aspect_ratio"));

    if (mlt_properties_get(b_props, "rescale.interp") == NULL || !strcmp(mlt_properties_get(b_props, "rescale.interp"), "none"))
        mlt_properties_set(b_props, "rescale.interp", mlt_properties_get(a_props, "rescale.interp"));

    // Compute position
    char *name = mlt_properties_get(transition_props, "_unique_id");
    mlt_position position = mlt_properties_get_position(a_props, name);

    // Get the aFrame image, we will write our output to it
    *format = mlt_image_rgb24;
    if ((error = mlt_frame_get_image(aFrame, image, format, width, height, 1)) != 0)
        return error;
    // Get the bFrame image, we won't write to it
    uint8_t *bImage = NULL;
    int bWidth = 0, bHeight = 0;
    if ((error = mlt_frame_get_image(bFrame, &bImage, format, &bWidth, &bHeight, 0)) != 0)
        return error;

    { // Scope the lock
        MLTWebVFX::ServiceLocker locker(MLT_TRANSITION_SERVICE(transition));
        if (!locker.initialize(*width, *height))
            return 1;

        //XXX fix this
        ChromixTransitionTask* task = (ChromixTransitionTask*)getTask(service);
        ChromixRawImage targetImage(*image, *width, *height);
        task->aTrackImage.set(*image, *width, *height);
        task->bTrackImage.set(bImage, bWidth, bHeight);
        error = task->renderToImageForPosition(targetImage, position);
        task->aTrackImage.set();
        task->bTrackImage.set();
    }

    return error;
}

static mlt_frame transitionProcess(mlt_transition transition, mlt_frame aFrame, mlt_frame bFrame) {
    char* name = mlt_properties_get(MLT_TRANSITION_PROPERTIES(transition), "_unique_id");
    mlt_properties_set_position(MLT_FRAME_PROPERTIES(aFrame), name, mlt_frame_get_position(aFrame));
    mlt_frame_push_service(aFrame, transition);
    mlt_frame_push_frame(aFrame, bFrame);
    mlt_frame_push_get_image(aFrame, transitionGetImage);
    return aFrame;
}

mlt_service MLTWebVFX::createTransition(const char* serviceName) {
    mlt_transition self = mlt_transition_new();
    if (self) {
        self->process = transitionProcess;
        // Video only transition
        mlt_properties_set_int(MLT_TRANSITION_PROPERTIES(self), "_transition_type", 1);
        return MLT_TRANSITION_SERVICE(self);
    }
    return 0;
}
