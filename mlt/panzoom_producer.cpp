// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_geometry.h>
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_frame.h>
}
#include <QImage>
#include <QPainter>
#include <QTransform>
#include "factory.h"

static const char* kPanzoomProducerPropertyName = "WebVfxPanzoomProducer";
static const char* kPanzoomPositionPropertyName = "webvfx.panzoom.position";
static const char* kPanzoomQImagePropertyName = "webvfx.panzoom.QImage";
static const char* kPanzoomGeometryPropertyName = "webvfx.panzoom.Geometry";

static QTransform computeTransform(mlt_producer producer, mlt_properties frameProperties, QImage* image) {
    mlt_properties producerProperties = MLT_PRODUCER_PROPERTIES(producer);
    mlt_geometry geometry = static_cast<mlt_geometry>(mlt_properties_get_data(producerProperties, kPanzoomGeometryPropertyName, NULL));

    if (!geometry) {
        int nwidth = mlt_properties_get_int(frameProperties, "normalised_width");
        int nheight = mlt_properties_get_int(frameProperties, "normalised_height");
        char* spec = mlt_properties_get(producerProperties, "geometry");
        geometry = mlt_geometry_init();
        mlt_geometry_parse(geometry, spec, mlt_producer_get_length(producer),
                           nwidth, nheight);
        mlt_properties_set_data(producerProperties,
                                kPanzoomGeometryPropertyName, geometry, 0,
                                reinterpret_cast<mlt_destructor>(mlt_geometry_close), NULL);
    }

    mlt_position position = mlt_properties_get_position(frameProperties, kPanzoomPositionPropertyName);
    struct mlt_geometry_item_s item;
    mlt_geometry_fetch(geometry, &item, position);

    // Compute scale to "meet" geometry
    float scaleWidth = item.w / image->width();
    float scaleHeight = item.h / image->height();
    float scale = qMin(scaleWidth, scaleHeight);

    // If aspect ratio differs, need to center image
    if (scaleWidth > scaleHeight)
        item.x = (item.w - scaleHeight * image->width()) / 2.0;
    else if (scaleHeight > scaleWidth)
        item.y = (item.h - scaleWidth * image->height()) / 2.0;

    return QTransform::fromTranslate(item.x, item.y).scale(scale, scale);
}

static int producerGetImage(mlt_frame frame, uint8_t **buffer, mlt_image_format *format, int *width, int *height, int /*writable*/) {
    int error = 0;

    // Obtain properties of frame
    mlt_properties properties = MLT_FRAME_PROPERTIES(frame);

    // Obtain the producer for this frame
    mlt_producer producer = (mlt_producer)mlt_properties_get_data(properties, kPanzoomProducerPropertyName, NULL);

    // Allocate the image
    //XXX support requested format?
    //XXX need to memset this?
    *format = mlt_image_rgb24;
    int size = *width * *height * 3;
    *buffer = (uint8_t*)mlt_pool_alloc(size);
    if (!*buffer)
        return 1;
    //XXX can we optimize this? i.e. don't need to clear if painting over it
    memset(*buffer, 0, size);

    // Update the frame
    mlt_frame_set_image(frame, *buffer, size, mlt_pool_release);
    mlt_properties_set_int(properties, "width", *width);
    mlt_properties_set_int(properties, "height", *height);

    mlt_properties producerProperties = MLT_PRODUCER_PROPERTIES(producer);
    QImage* image = static_cast<QImage*>(mlt_properties_get_data(producerProperties, kPanzoomQImagePropertyName, NULL));

    QImage targetImage(static_cast<uchar*>(*buffer), *width, *height, *width * 3, QImage::Format_RGB888);
    QPainter painter(&targetImage);
    painter.setTransform(computeTransform(producer, properties, image));
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(QPoint(0, 0), *image);

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
        mlt_properties_set_data(properties, kPanzoomProducerPropertyName, producer, 0, NULL, NULL);

        // Update timecode on the frame we're creating
        mlt_position position = mlt_producer_position(producer);
        mlt_frame_set_position(*frame, position);
        mlt_properties_set_position(properties, kPanzoomPositionPropertyName, position);

        // Set producer-specific frame properties
        mlt_properties_set_int(properties, "progressive", 1);
        mlt_properties_set_double(properties, "aspect_ratio", mlt_properties_get_double(producer_props, "aspect_ratio"));

        // Push the get_image method
        mlt_frame_push_get_image(*frame, producerGetImage);
    }

    // Calculate the next timecode
    mlt_producer_prepare_next(producer);

    return 0;
}

static void destroyQImage(QImage* image) {
    delete image;
}

void* MLTWebVfx::createPanzoomProducer(mlt_profile profile, mlt_service_type, const char*, const void* fileName) {
    mlt_producer self = mlt_producer_new(profile);
    if (self) {
        mlt_properties properties = MLT_PRODUCER_PROPERTIES(self);
        self->get_frame = getFrame;
        if (!fileName)
            fileName = mlt_properties_get(properties, "resource");
        QImage* image = new QImage(static_cast<const char*>(fileName));
        mlt_properties_set_data(properties, kPanzoomQImagePropertyName, image, 0, reinterpret_cast<mlt_destructor>(destroyQImage), NULL);
        if (!image || image->isNull()) {
            mlt_producer_close(self);
            self = 0;
        }
        mlt_properties_set(properties, "geometry", "0/0:100%x100%");
    }
    return self;
}
