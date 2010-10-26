#include <mlt/framework/mlt_filter.h>
#include <mlt/framework/mlt_frame.h>
#include "chromix_helper.h"

static int filter_get_image(mlt_frame this, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable) {
    // Get the filter
    mlt_filter filter = mlt_frame_pop_service(this);

    // Compute position
    mlt_position in = mlt_filter_get_in(filter);
	mlt_position out = mlt_filter_get_out(filter);
	mlt_position time = mlt_frame_get_position(this);
	double position = (double)(time - in) / (double)(out - in + 1);

    // Get the image
    *format = mlt_image_rgb24a;
    int error = mlt_frame_get_image(this, image, format, width, height, 1);
    if (error != 0)
        return error;
    //XXX need to get track name from properties
    error = chromix_set_image(MLT_FILTER_PROPERTIES(filter), "video", *image, *width, *height);
    if (error != 0)
        return error;
    error = chromix_render(MLT_FILTER_PROPERTIES(filter), position, *image, *width, *height);
    return error;
}

/** Filter processing.
 */
mlt_frame chromix_filter_process(mlt_filter this, mlt_frame frame) {
    // Push the frame filter
    mlt_frame_push_service(frame, this);
    mlt_frame_push_get_image(frame, filter_get_image);

    return frame;
}
