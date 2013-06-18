// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_frame.h>
}
#include <webvfx/image.h>
#include "factory.h"
#include "service_locker.h"
#include "service_manager.h"

static const char* kWebVfxProducerPropertyName = "WebVfxProducer";
static const char* kWebVfxPositionPropertyName = "webvfx.position";

static int producerGetImage(mlt_frame frame, uint8_t **buffer, mlt_image_format *format, int *width, int *height, int /*writable*/) {
    int error = 0;
    mlt_properties properties = MLT_FRAME_PROPERTIES(frame);
    mlt_producer producer = (mlt_producer)mlt_properties_get_data(properties, kWebVfxProducerPropertyName, NULL);
    mlt_properties producer_props = MLT_PRODUCER_PROPERTIES(producer);
    int size;
    int bpp;
    {
	MLTWebVfx::ServiceLocker locker(MLT_PRODUCER_SERVICE(producer));
	if (!locker.initialize(*width, *height))
	    return 1;
	    
	if ( mlt_properties_get_int( producer_props, "transparent") ) {
	    *format = mlt_image_rgb24a;
	    bpp = 4;
	}
	else {
	    *format = mlt_image_rgb24;
	    bpp = 3;
	}
	    
	size = *width * *height * bpp;
	*buffer = (uint8_t*)mlt_pool_alloc(size);
	
	// When not using transparency, thiss will make the background black...
	memset( *buffer, 255, size );
	WebVfx::Image outputImage(*buffer, *width, *height, size, bpp == 4);
	locker.getManager()->render(&outputImage,
                                    mlt_properties_get_position(properties, kWebVfxPositionPropertyName),
                                    mlt_producer_get_length(producer), bpp == 4);
	}
    mlt_frame_set_image(frame, *buffer, size, mlt_pool_release);
    if ( bpp == 4 ) {
	// Create the alpha channel
	int alpha_size = *width * *height;
	uint8_t *alpha = (uint8_t *)mlt_pool_alloc( alpha_size );
	// Initialise the alpha
	memset( alpha, 255, alpha_size );
	mlt_frame_set_alpha( frame, alpha, alpha_size, mlt_pool_release );
    }

    mlt_properties_set_double( properties, "aspect_ratio", mlt_properties_get_double( producer_props, "aspect_ratio" ) );
    mlt_properties_set_int( properties, "meta.media.width", *width );
    mlt_properties_set_int( properties, "meta.media.height", *height );

    return error;
}

static int getFrame(mlt_producer producer, mlt_frame_ptr frame, int /*index*/) {
    // Generate a frame
    *frame = mlt_frame_init(MLT_PRODUCER_SERVICE(producer));

    if (*frame) {
        // Obtain properties of frame and producer
        mlt_properties properties = MLT_FRAME_PROPERTIES(*frame);

        // Obtain properties of producer
        mlt_properties producer_props = MLT_PRODUCER_PROPERTIES(producer);

        // Set the producer on the frame properties
        mlt_properties_set_data(properties, kWebVfxProducerPropertyName, producer, 0, NULL, NULL);

        // Update timecode on the frame we're creating
        mlt_position position = mlt_producer_position(producer);
        mlt_frame_set_position(*frame, position);
        mlt_properties_set_position(properties, kWebVfxPositionPropertyName, position);

        // Set producer-specific frame properties
        mlt_properties_set_int(properties, "progressive", 1);
	mlt_profile profile = mlt_service_profile( MLT_PRODUCER_SERVICE( producer ) );
	mlt_properties_set_double( properties, "aspect_ratio", mlt_profile_sar( profile ) );
        mlt_properties_set_double(properties, "aspect_ratio", mlt_properties_get_double(producer_props, "aspect_ratio"));

        // Push the get_image method
        mlt_frame_push_get_image(*frame, producerGetImage);
    }

    // Calculate the next timecode
    mlt_producer_prepare_next(producer);

    return 0;
}

static void producer_close( mlt_producer parent )
{
	// Close the parent
	parent->close = NULL;
	mlt_producer_close( parent );
	// Free the memory
	free( parent );
}

mlt_service MLTWebVfx::createProducer(mlt_profile profile) {
    mlt_producer producer = mlt_producer_new(profile);
    if (producer) {
        producer->get_frame = getFrame;
	producer->close = (mlt_destructor) producer_close;
	mlt_properties properties = MLT_PRODUCER_PROPERTIES( producer );
	mlt_properties_set_double( properties, "aspect_ratio", mlt_profile_sar( profile ) );
        return MLT_PRODUCER_SERVICE(producer);
    }
    return 0;
}
