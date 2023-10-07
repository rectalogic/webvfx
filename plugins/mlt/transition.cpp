// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
#include <framework/mlt_frame.h> // for mlt_frame_get_image, mlt_frame_s, mlt_frame_pop_frame, mlt_frame_pop_service, mlt_frame_push_frame, mlt_frame_push_get_image, mlt_frame_push_service
#include <framework/mlt_image.h> // for mlt_image_set_values, mlt_image_s
#include <framework/mlt_properties.h> // for mlt_properties_set_int
#include <framework/mlt_service.h> // for mlt_service_s
#include <framework/mlt_transition.h> // for mlt_transition_s, mlt_transition_get_length, mlt_transition_get_position, mlt_transition_new, MLT_TRANSITION_SERVICE, MLT_TRANSITION_PROPERTIES
#include <framework/mlt_types.h> // for mlt_frame, mlt_transition, mlt_position, mlt_image_format, mlt_image_rgba, mlt_service
}
#include "factory.h" // for createTransition
#include "service_locker.h" // for ServiceLocker
#include "service_manager.h" // for ServiceManager
#include "vfxpipe.h" // for SourceVideoFrame, VideoFrameFormat, VideoFrameFormat::PixelFormat, VideoFrameFormat::RGBA32
#include <cstddef> /* IWYU pragma: keep */ /* IWYU pragma: no_include <ext/type_traits> */ // for byte, NULL
#include <stdint.h> // for uint8_t

static int transitionGetImage(mlt_frame aFrame, uint8_t** image, mlt_image_format* format, int* width, int* height, int /*writable*/)
{
    int error = 0;

    mlt_frame bFrame = mlt_frame_pop_frame(aFrame);
    mlt_transition transition = (mlt_transition)mlt_frame_pop_service(aFrame);

    mlt_position position = mlt_transition_get_position(transition, aFrame);
    mlt_position length = mlt_transition_get_length(transition);

    // Get the aFrame image, we will write our output to it
    *format = mlt_image_rgba;
    if ((error = mlt_frame_get_image(aFrame, image, format, width, height, 1)) != 0)
        return error;
    // Get the bFrame image, we won't write to it
    uint8_t* bImage = NULL;
    int bWidth = 0, bHeight = 0;
    if ((error = mlt_frame_get_image(bFrame, &bImage, format, &bWidth, &bHeight, 0)) != 0)
        return error;

    { // Scope the lock
        WebVfxPlugin::ServiceLocker locker(MLT_TRANSITION_SERVICE(transition));
        if (!locker.initialize(length))
            return 1;

        WebVfxPlugin::ServiceManager* manager = locker.getManager();
        mlt_image_s renderedImage;
        mlt_image_set_values(&renderedImage, *image, *format, *width, *height);
        mlt_image_s targetImage;
        mlt_image_set_values(&targetImage, bImage, *format, bWidth, bHeight);
        VfxPipe::SourceVideoFrame vfxSourceFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, *width, *height, reinterpret_cast<const std::byte*>(renderedImage.data));
        VfxPipe::SourceVideoFrame vfxTargetFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, bWidth, bHeight, reinterpret_cast<const std::byte*>(targetImage.data));
        manager->render(&vfxSourceFrame, &vfxTargetFrame, &renderedImage, position);
    }

    return error;
}

static mlt_frame transitionProcess(mlt_transition transition, mlt_frame aFrame, mlt_frame bFrame)
{
    mlt_frame_push_service(aFrame, transition);
    mlt_frame_push_frame(aFrame, bFrame);
    mlt_frame_push_get_image(aFrame, transitionGetImage);
    return aFrame;
}

mlt_service WebVfxPlugin::createTransition()
{
    mlt_transition self = mlt_transition_new();
    if (self) {
        self->process = transitionProcess;
        // Video only transition
        mlt_properties_set_int(MLT_TRANSITION_PROPERTIES(self), "_transition_type", 1);
        return MLT_TRANSITION_SERVICE(self);
    }
    return 0;
}
