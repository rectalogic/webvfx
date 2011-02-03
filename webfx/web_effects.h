// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_EFFECTS_H_
#define WEBFX_WEB_EFFECTS_H_

#include <string>
#include <webfx/web_image.h>
#include <webfx/web_parameters.h>

namespace WebFX
{

class WebEffects
{
public:
    //XXX expose src/dst/extra image metadata from JS
    //XXX set raw image data - can we do that in bulk then marshal all over to UI thread to render?

    // A Parameters implementation may be supplied to provide parameters to the mix.
    // WebEffects will take ownership of parameters.
    virtual bool initialize(const std::string& url, int width, int height, WebParameters* parameters = 0) = 0;
    virtual WebImage render(double time, int width, int height) = 0;
    virtual void destroy() = 0;

protected:
    WebEffects() {};
    virtual ~WebEffects() = 0;
};

}

#endif
