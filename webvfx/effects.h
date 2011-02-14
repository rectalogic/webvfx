// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_EFFECTS_H_
#define WEBVFX_EFFECTS_H_

#include <string>
#include <map>
#include <webvfx/image.h>
#include <webvfx/parameters.h>

namespace WebVfx
{

// Instances of this class can be accessed from any thread,
// but the class is not threadsafe - access should be serialized.
class Effects
{
public:
    enum ImageType { SourceImageType=1, TargetImageType, ExtraImageType };

    typedef std::map<const std::string, ImageType> ImageTypeMap;

    // Return a map mapping image names the page content uses to imge type.
    // XXX should we expose opaque "cooke" image name that holds QString internally? (and with a toString() method) - so caller can efficiently set image using cookie name
    virtual const ImageTypeMap& getImageTypeMap() = 0;
    // Return a Image of the given size that can be written to.
    virtual Image getImage(const std::string& name, int width, int height) = 0;
    //XXX need to return an error code?
    virtual const Image render(double time, int width, int height) = 0;
    virtual void destroy() = 0;

protected:
    Effects() {};
    virtual ~Effects() = 0;
};

}

#endif
