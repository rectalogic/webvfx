// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt_geometry.h>
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_frame.h>
    #include <mlt/framework/mlt_log.h>
}
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QSize>
#include <QTransform>
#include "factory.h"

static const char* kPanzoomFilenamePropertyName = "webvfx.panzoom.filename";
static const char* kPanzoomProducerPropertyName = "WebVfxPanzoomProducer";
static const char* kPanzoomPositionPropertyName = "webvfx.panzoom.position";
static const char* kPanzoomQImagePropertyName = "webvfx.panzoom.QImage";
static const char* kPanzoomGeometryPropertyName = "webvfx.panzoom.Geometry";


static void destroyQImage(QImage* image) {
    delete image;
}

static QSize computeMaxSize(mlt_geometry geometry) {
    QSize size(0, 0);
    struct mlt_geometry_item_s item;
    int position = 0;
    while (!mlt_geometry_next_key(geometry, &item, position)) {
        if (item.w > size.width())
            size.setWidth(item.w);
        if (item.h > size.height())
            size.setHeight(item.h);
        position = item.frame + 1;
    }
    return size;
}

static mlt_geometry getGeometry(mlt_producer producer, mlt_properties frameProperties) {
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
    return geometry;
}

static QImage* getPrescaledSourceImage(mlt_producer producer, mlt_geometry geometry) {
    mlt_properties producerProperties = MLT_PRODUCER_PROPERTIES(producer);
    QImage* image = static_cast<QImage*>(mlt_properties_get_data(producerProperties, kPanzoomQImagePropertyName, NULL));
    if (!image) {
        const char* fileName = mlt_properties_get(producerProperties, kPanzoomFilenamePropertyName);
        if (!fileName)
            fileName = mlt_properties_get(producerProperties, "resource");

        image = new QImage(fileName);
        if (image) {
            mlt_properties_set_data(producerProperties, kPanzoomQImagePropertyName, image, 0, reinterpret_cast<mlt_destructor>(destroyQImage), NULL);
            if (image->isNull()) {
                QImageReader reader(fileName);
                reader.setDecideFormatFromContent(true);
                *image = reader.read();
                if (image->isNull()) {
                    mlt_log(MLT_PRODUCER_SERVICE(producer), MLT_LOG_ERROR,
                            "Failed to load QImage '%s'\n", fileName);
                    return NULL;
                }
            }

            // Prescale image down to max size needed
            QSize maxSize(computeMaxSize(geometry));
            QSize imageSize(image->size());
            if (maxSize.width() < imageSize.width() && maxSize.height() < imageSize.height())
                *image = image->scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        else {
            mlt_log(MLT_PRODUCER_SERVICE(producer), MLT_LOG_ERROR,
                    "Failed to create QImage\n");
            return NULL;
        }
    }
    return image;
}

static int producerGetImage(mlt_frame frame, uint8_t **buffer, mlt_image_format *format, int *width, int *height, int /*writable*/) {
    int error = 0;

    // Obtain properties of frame
    mlt_properties properties = MLT_FRAME_PROPERTIES(frame);

    // Obtain the producer for this frame
    mlt_producer producer = static_cast<mlt_producer>(mlt_properties_get_data(properties, kPanzoomProducerPropertyName, NULL));

    // Allocate the image
    *format = mlt_image_rgb24;
    int size = *width * *height * 3;
    *buffer = (uint8_t*)mlt_pool_alloc(size);
    if (!*buffer)
        return 1;

    // Update the frame
    mlt_frame_set_image(frame, *buffer, size, mlt_pool_release);
    mlt_properties_set_int(properties, "width", *width);
    mlt_properties_set_int(properties, "height", *height);

    mlt_geometry geometry = getGeometry(producer, properties);
    QImage* image = getPrescaledSourceImage(producer, geometry);
    if (!image)
        return 1;

    struct mlt_geometry_item_s item;
    mlt_position position = mlt_properties_get_position(properties, kPanzoomPositionPropertyName);
    mlt_geometry_fetch(geometry, &item, position);

    // Compute scale to "meet" the geometry rect
    float scaleWidth = item.w / image->width();
    float scaleHeight = item.h / image->height();
    float scale = qMin(scaleWidth, scaleHeight);

    // If aspect ratio differs, need to center image
    if (scaleWidth > scaleHeight)
        item.x += (item.w - scaleHeight * image->width()) / 2.0;
    else if (scaleHeight > scaleWidth)
        item.y += (item.h - scaleWidth * image->height()) / 2.0;

    QImage targetImage(static_cast<uchar*>(*buffer), *width, *height, *width * 3, QImage::Format_RGB888);

    QTransform tx(QTransform::fromTranslate(item.x, item.y).scale(scale, scale));

    // Clear buffer if image doesn't fill it
    QRect rect(tx.mapRect(image->rect()));
    if (!rect.contains(targetImage.rect()))
        memset(*buffer, 0, size);

    QPainter painter(&targetImage);
    painter.setTransform(tx);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(QPointF(0, 0), *image);

    return error;
}

static int getFrame(mlt_producer producer, mlt_frame_ptr frame, int /*index*/) {
    // Generate a frame
    *frame = mlt_frame_init(MLT_PRODUCER_SERVICE(producer));

    if (*frame) {
        // Obtain properties of frame and producer
        mlt_properties properties = MLT_FRAME_PROPERTIES(*frame);

        // Obtain properties of producer
        mlt_properties producerProperties = MLT_PRODUCER_PROPERTIES(producer);

        // Set the producer on the frame properties
        mlt_properties_set_data(properties, kPanzoomProducerPropertyName, producer, 0, NULL, NULL);

        // Update timecode on the frame we're creating
        mlt_position position = mlt_producer_position(producer);
        mlt_frame_set_position(*frame, position);
        mlt_properties_set_position(properties, kPanzoomPositionPropertyName, position);

        // Set producer-specific frame properties
        mlt_properties_set_int(properties, "progressive", 1);
        mlt_properties_set_double(properties, "aspect_ratio", mlt_properties_get_double(producerProperties, "aspect_ratio"));

        // Push the get_image method
        mlt_frame_push_get_image(*frame, producerGetImage);
    }

    // Calculate the next timecode
    mlt_producer_prepare_next(producer);

    return 0;
}

void* MLTWebVfx::createPanzoomProducer(mlt_profile profile, mlt_service_type, const char*, const void* fileName) {
    mlt_producer self = mlt_producer_new(profile);
    if (self) {
        mlt_properties properties = MLT_PRODUCER_PROPERTIES(self);
        self->get_frame = getFrame;
        if (fileName) {
            mlt_properties_set(properties, kPanzoomFilenamePropertyName,
                               static_cast<const char *>(fileName));
        }
        mlt_properties_set(properties, "geometry", "0/0:100%x100%");
    }
    return self;
}
