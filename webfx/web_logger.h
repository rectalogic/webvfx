// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_LOGGER_H_
#define WEBFX_WEB_LOGGER_H_

#include <string>

namespace WebFX {

class WebLogger
{
public:
    virtual ~WebLogger() = 0;
    //XXX should add log level enum
    virtual void log(const std::string& msg) = 0;
};

void log(const std::string& msg);

}

#endif
