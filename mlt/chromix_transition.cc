// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_transition.h>
    #include <mlt/framework/mlt_frame.h>
}
#include "chromix_task.h"


class ChromixTransitionTask : public ChromixTask {
public:

    static int transitionGetImage(mlt_frame a_frame, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable) {
        int error = 0;

        mlt_frame b_frame = mlt_frame_pop_frame(a_frame);
        mlt_transition transition = (mlt_transition)mlt_frame_pop_service(a_frame);
        mlt_properties transition_props = MLT_TRANSITION_PROPERTIES(transition);
        mlt_properties a_props = MLT_FRAME_PROPERTIES(a_frame);
        mlt_properties b_props = MLT_FRAME_PROPERTIES(b_frame);

        // Set consumer_aspect_ratio for a and b frame
        if (mlt_properties_get_double(a_props, "aspect_ratio") == 0.0)
            mlt_properties_set_double(a_props, "aspect_ratio", mlt_properties_get_double(a_props, "consumer_aspect_ratio"));
        if (mlt_properties_get_double(b_props, "aspect_ratio") == 0.0)
            mlt_properties_set_double(b_props, "aspect_ratio", mlt_properties_get_double(a_props, "consumer_aspect_ratio"));
        mlt_properties_set_double(b_props, "consumer_aspect_ratio", mlt_properties_get_double(a_props, "consumer_aspect_ratio"));

        if (mlt_properties_get(b_props, "rescale.interp") == NULL || !strcmp(mlt_properties_get(b_props, "rescale.interp"), "none"))
            mlt_properties_set(b_props, "rescale.interp", mlt_properties_get(a_props, "rescale.interp"));

        // Compute time
        mlt_position in = mlt_transition_get_in(transition);
        mlt_position length = mlt_transition_get_out(transition) - in + 1;
        char *name = mlt_properties_get(transition_props, "_unique_id");
        mlt_position position = mlt_properties_get_position(a_props, name);
        double time = (double)(position - in) / (double)length;

        // Get the a_frame image, we will write our output to it
        *format = mlt_image_rgb24a;
        if ((error = mlt_frame_get_image(a_frame, image, format, width, height, 1)) != 0)
            return error;
        // Get the b_frame image, we won't write to it
        uint8_t *b_image = NULL;
        int b_width = 0, b_height = 0;
        if ((error = mlt_frame_get_image(b_frame, &b_image, format, &b_width, &b_height, 0)) != 0)
            return error;

        { // Scope the ServiceLock
            mlt_service service = MLT_TRANSITION_SERVICE(transition);
            ServiceLock lock(service);

            ChromixTransitionTask *task = (ChromixTransitionTask*)getTask(service);
            if (!task)
                task = new ChromixTransitionTask(service);

            ChromixRawImage targetImage(*image, *width, *height);
            task->aTrackImage.set(*image, *width, *height);
            task->bTrackImage.set(b_image, b_width, b_height);
            error = task->renderToImageForTime(targetImage, time);
            task->aTrackImage.set();
            task->bTrackImage.set();
        }

        return error;
    }

protected:
    ChromixTransitionTask(mlt_service service) : ChromixTask(service) {}

    int performTask() {
        //XXX lookup param - map track to WTF::String
        //XXX lookup track property to get the WTF::String mixrender name for the image
        //XXX need a map of name to ChromixRawImage - maintain in ChromixTask, clear after each render
        int result = setImageForName(aTrackImage, "a_track");//XXX
        if (result == 0)
            result = setImageForName(bTrackImage, "b_track");//XXX
        return result;
    };

private:
    ChromixRawImage aTrackImage;
    ChromixRawImage bTrackImage;
};

mlt_frame chromix_transition_process(mlt_transition transition, mlt_frame a_frame, mlt_frame b_frame) {
    char *name = mlt_properties_get(MLT_TRANSITION_PROPERTIES(transition), "_unique_id");
    mlt_properties_set_position(MLT_FRAME_PROPERTIES(a_frame), name, mlt_frame_get_position(a_frame));
    mlt_frame_push_service(a_frame, transition);
    mlt_frame_push_frame(a_frame, b_frame);
    mlt_frame_push_get_image(a_frame, ChromixTransitionTask::transitionGetImage);
    return a_frame;
}
