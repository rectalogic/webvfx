#include <mlt/framework/mlt_filter.h>
#include <mlt/framework/mlt_frame.h>
#include "chromix_helper.h"

static int filter_get_image(mlt_frame this, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable) {
    int error = 0;
    void* context = chromix_context_new();

    // Get the filter
    mlt_filter filter = mlt_frame_pop_service(this);

    if ((error = chromix_initialize_service_properties(MLT_FILTER_PROPERTIES(filter))) != 0)
        goto finished;

    // Compute position
    //XXX this is wrong - in/out are always 0 - maybe have to deal with this in chromix, or always specify out in params?
    mlt_position in = mlt_filter_get_in(filter);
    mlt_position length = mlt_filter_get_out(filter) - in + 1;
	mlt_position time = mlt_frame_get_position(this);
    double position = (double)(time - in) / (double)length;

    // Get the image
    *format = mlt_image_rgb24a;
    if ((error = mlt_frame_get_image(this, image, format, width, height, 1)) != 0)
        goto finished;
    //XXX need to get track name from properties
    if ((error = chromix_set_image(MLT_FILTER_PROPERTIES(filter), "video", *image, *width, *height)) != 0)
        goto finished;
    if ((error = chromix_render(MLT_FILTER_PROPERTIES(filter), position, *image, *width, *height)) != 0)
        goto finished;

finished:
    chromix_context_close(context);
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
