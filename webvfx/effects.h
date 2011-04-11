// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_EFFECTS_H_
#define WEBVFX_EFFECTS_H_

#include <QMap>
#include <QString>
#include <webvfx/image.h>
#include <webvfx/parameters.h>

class QString;

namespace WebVfx
{

// Instances of this class can be accessed from any thread,
// but the class is not threadsafe - access should be serialized.
class Effects
{
public:
    enum ImageType { SourceImageType=1, TargetImageType, ExtraImageType };

    typedef QMap<QString, ImageType> ImageTypeMap;
    typedef QMapIterator<QString, ImageType> ImageTypeMapIterator;

    // Return a map mapping image names the page content uses to imge type.
    virtual const ImageTypeMap& getImageTypeMap() = 0;
    // Set an Image for the given name. Must be valid until render() is called.
    virtual void setImage(const QString& name, Image* image) = 0;
    virtual bool render(double time, Image* renderImage) = 0;
    virtual void destroy() = 0;

protected:
    Effects() {};
    virtual ~Effects() = 0;
};

}

#endif
