// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
#include <framework/mlt_filter.h> // for mlt_filter_s, mlt_filter_get_length, mlt_filter_get_position, mlt_filter_new, MLT_FILTER_SERVICE
#include <framework/mlt_frame.h> // for mlt_frame_get_image, mlt_frame_pop_service, mlt_frame_push_get_image, mlt_frame_push_service, mlt_frame_s
#include <framework/mlt_image.h> // for mlt_image_set_values, mlt_image_s
#include <framework/mlt_service.h> // for mlt_service_s
#include <framework/mlt_types.h> // for mlt_filter, mlt_frame, mlt_position, mlt_image_format, mlt_image_rgba, mlt_service
}
#include "factory.h" // for createFilter
#include "service_locker.h" // for ServiceLocker
#include "service_manager.h" // for ServiceManager
#include <cstddef> /* IWYU pragma: keep */ /* IWYU pragma: no_include <ext/type_traits> */ // for byte
#include <stdint.h> // for uint8_t
#include <vfxpipe.h> // for SourceVideoFrame, VideoFrameFormat

static int filterGetImage(mlt_frame frame, uint8_t** image, mlt_image_format* format, int* width, int* height, int /*writable*/)
{
    int error = 0;

    // Get the filter
    mlt_filter filter = (mlt_filter)mlt_frame_pop_service(frame);

    mlt_position position = mlt_filter_get_position(filter, frame);
    mlt_position length = mlt_filter_get_length(filter);

    // Get the source image, we will also write our output to it
    *format = mlt_image_rgba;
    if ((error = mlt_frame_get_image(frame, image, format, width, height, 1)) != 0)
        return error;

    { // Scope the lock
        WebVfxPlugin::ServiceLocker locker(MLT_FILTER_SERVICE(filter));
        if (!locker.initialize(length))
            return 1;

        WebVfxPlugin::ServiceManager* manager = locker.getManager();
        mlt_image_s renderedImage;
        mlt_image_set_values(&renderedImage, *image, *format, *width, *height);
        VfxPipe::SourceVideoFrame vfxFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, *width, *height, reinterpret_cast<const std::byte*>(*image));
        manager->render(&vfxFrame, nullptr, &renderedImage, position);
    }

    return error;
}

static mlt_frame filterProcess(mlt_filter filter, mlt_frame frame)
{
    // Push the frame filter
    mlt_frame_push_service(frame, filter);
    mlt_frame_push_get_image(frame, filterGetImage);

    return frame;
}

mlt_service WebVfxPlugin::createFilter()
{
    mlt_filter self = mlt_filter_new();
    if (self) {
        self->process = filterProcess;
        return MLT_FILTER_SERVICE(self);
    }
    return 0;
}
