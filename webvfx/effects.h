// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_EFFECTS_H_
#define WEBVFX_EFFECTS_H_

#include <QMap>
#include <QString>


namespace WebVfx
{

class Effects
{
public:
    /*!
     * @brief Describes the type of an image.
     */
    enum ImageType {
        /*!
         * The source (origin/from) image in a transition,
         * or the image being processed in a filter.
         */
        SourceImageType=1,
        /*!
         * The target (destination/to) image in a transition.
         */
        TargetImageType,
        /*!
         * An extra image not directly participating in a
         *   filter or transition.
         */
        ExtraImageType
    };

    typedef QMap<QString, ImageType> ImageTypeMap;
    typedef QMapIterator<QString, ImageType> ImageTypeMapIterator;
};

}

#endif
