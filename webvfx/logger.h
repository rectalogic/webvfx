// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_LOGGER_H_
#define WEBVFX_LOGGER_H_

#include <QString>

namespace WebVfx {

class Logger
{
public:
    virtual ~Logger() = 0;
    //XXX should add log level enum
    virtual void log(const QString& msg) = 0;
};

void log(const QString& msg);

}

#endif
