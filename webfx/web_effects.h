// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_EFFECTS_H_
#define WEBFX_WEB_EFFECTS_H_

#include <string>
#include <map>
#include <webfx/web_image.h>
#include <webfx/web_parameters.h>

namespace WebFX
{

// Instances of this class can be accessed from any thread,
// but the class is not threadsafe - access should be serialized.
class WebEffects
{
public:
    enum ImageType { SrcImageType=1, DstImageType, ExtraImageType };

    typedef std::map<const std::string, ImageType> ImageTypeMap;

    // A Parameters implementation may be supplied to provide parameters to the page.
    // WebEffects will take ownership of parameters.
    virtual bool initialize(const std::string& url, int width, int height, WebParameters* parameters = 0) = 0;
    // Return a map mapping image names the page content uses to imge type.
    // XXX should we expose opaque "cooke" image name that holds QString internally? (and with a toString() method) - so caller can efficiently set image using cookie name
    virtual const ImageTypeMap& getImageTypeMap() = 0;
    // Return a WebImage of the given size that can be written to.
    virtual WebImage getImage(const std::string& name, int width, int height) = 0;
    virtual const WebImage render(double time, int width, int height) = 0;
    virtual void destroy() = 0;

protected:
    WebEffects() {};
    virtual ~WebEffects() = 0;
};

}

#endif
