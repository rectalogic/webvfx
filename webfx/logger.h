// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_LOGGER_H_
#define WEBFX_LOGGER_H_

#include <string>

namespace WebFX {

class Logger
{
public:
    virtual ~Logger() = 0;
    virtual void log(const std::string& msg) = 0;
};

void log(const std::string& msg);

}

#endif
