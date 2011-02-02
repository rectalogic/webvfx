// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_MIX_KIT_H_
#define CHROMIX_MIX_KIT_H_

#include <string>
#include <chromix/parameters.h>

namespace Chromix
{

class MixKit
{
public:
    //XXX expose src/dst/extra image metadata from JS
    //XXX set raw image data - can we do that in bulk then marshal all over to UI thread to render?
    //XXX render
    //XXX need delegate to get named parameter values

    // A Parameters implementation may be supplied to provide parameters to the mix.
    // MixKit will take ownership of parameters.
    virtual bool initialize(const std::string& url, int width, int height, Parameters* parameters = 0) = 0;
    virtual void setSize(int width, int height) = 0;
    virtual void destroy() = 0;

protected:
    MixKit() {};
    virtual ~MixKit() = 0;
};

}

#endif
