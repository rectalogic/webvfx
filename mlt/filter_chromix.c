#include <mlt/framework/mlt_filter.h>
#include <mlt/framework/mlt_frame.h>
#include "chromix_helper.h"

static int filter_get_image( mlt_frame this, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable )
{
    RGB32 *background;
    unsigned char *diff;
    unsigned char *buffer;
    
    // Get the filter
    mlt_filter filter = mlt_frame_pop_service( this );
    
    // Get the image
    *format = mlt_image_rgb24a;
    int error = mlt_frame_get_image( this, image, format, width, height, 1 );
    
    // Only process if we have no error and a valid colour space
    if ( error == 0 )
    {
        // Get the "Burn the foreground" value
        int burn_foreground = mlt_properties_get_int( MLT_FILTER_PROPERTIES( filter ), "foreground" );
        int y_threshold = image_set_threshold_y(
                                                mlt_properties_get_int( MLT_FILTER_PROPERTIES( filter ), "threshold" ));
        
        // We'll process pixel by pixel
        int x = 0;
        int y = 0;
        int i;
        
        int video_width = *width;
        int video_height = *height;
        int video_area = video_width * video_height;
        // We need to create a new frame as this effect modifies the input
        RGB32 *dest = (RGB32*)*image;
        RGB32 *src = (RGB32*)*image;
        
        unsigned char v, w;
        RGB32 a, b;
        
        diff = mlt_properties_get_data( MLT_FILTER_PROPERTIES( filter ), 
                                       "_diff", NULL );
        if (diff == NULL)
        {
            diff = mlt_pool_alloc(video_area*sizeof(unsigned char));
            mlt_properties_set_data( MLT_FILTER_PROPERTIES( filter ), "_diff", 
                                    diff, video_area*sizeof(unsigned char), mlt_pool_release, NULL );
        }
        
        buffer = mlt_properties_get_data( MLT_FILTER_PROPERTIES( filter ), 
                                         "_buffer", NULL );
        if (buffer == NULL)
        {
            buffer = mlt_pool_alloc(video_area*sizeof(unsigned char));
            memset(buffer, 0, video_area*sizeof(unsigned char));
            mlt_properties_set_data( MLT_FILTER_PROPERTIES( filter ), "_buffer", 
                                    buffer, video_area*sizeof(unsigned char), mlt_pool_release, NULL );
        }
        
        
        if (burn_foreground == 1) {
            /* to burn the foreground, we need a background */
            background = mlt_properties_get_data( MLT_FILTER_PROPERTIES( filter ), 
                                                 "_background", NULL );
            if (background == NULL)
            {
                background = mlt_pool_alloc(video_area*sizeof(RGB32));
                image_bgset_y(background, src, video_area, y_threshold);
                mlt_properties_set_data( MLT_FILTER_PROPERTIES( filter ), "_background", 
                                        background, video_area*sizeof(RGB32), mlt_pool_release, NULL );
            }
        }
        
        if (burn_foreground == 1) {
            image_bgsubtract_y(diff, background, src, video_area, y_threshold);
        } else {
            /* default */
            image_y_over(diff, src, video_area, y_threshold);
        }
        
        for(x=1; x<video_width-1; x++) {
            v = 0;
            for(y=0; y<video_height-1; y++) {
                w = diff[y*video_width+x];
                buffer[y*video_width+x] |= v ^ w;
                v = w;
            }
        }
        for(x=1; x<video_width-1; x++) {
            i = video_width + x;
            for(y=1; y<video_height; y++) {
                v = buffer[i];
                if(v<Decay)
                    buffer[i-video_width] = 0;
                else
                    buffer[i-video_width+fastrand()%3-1] = v - (fastrand()&Decay);
                i += video_width;
            }
        }
        
        i = 1;
        for(y=0; y<video_height; y++) {
            for(x=1; x<video_width-1; x++) {
                /* FIXME: endianess? */
                a = (src[i] & 0xfefeff) + palette[buffer[i]];
                b = a & 0x1010100;
                dest[i] = a | (b - (b >> 8));
                i++;
            }
            i += 2;
        }
    }
    
    return error;
}

/** Filter processing.
 */
static mlt_frame filter_process(mlt_filter this, mlt_frame frame) {
    // Push the frame filter
    mlt_frame_push_service(frame, this);
    mlt_frame_push_get_image(frame, filter_get_image);

    return frame;
}

/** Constructor for the filter.
 */
mlt_filter filter_chromix_init(mlt_profile profile, mlt_service_type type, const char *id, char *arg) {
    mlt_filter this = mlt_filter_new();
    if (this != NULL) {
        this->process = filter_process;
        //XXX need to pass html file too - is it arg above?
        if (!chromix_initialize_properties(MLT_FILTER_PROPERTIES(this))) {
            mlt_filter_close(this);
            return NULL;
        }
        //XXX set properties we get from filter - need to create MixRender using arg as url
        //XXXmlt_properties_set( MLT_FILTER_PROPERTIES( this ), "foreground", "0" );
        //XXXmlt_properties_set( MLT_FILTER_PROPERTIES( this ), "threshold", MAGIC_THRESHOLD );
    }
    return this;
}