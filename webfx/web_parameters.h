// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_PARAMETERS_H_
#define WEBFX_WEB_PARAMETERS_H_

#include <string>

namespace WebFX
{

class WebParameters
{
public:
    WebParameters() {};
    virtual ~WebParameters() = 0;
    virtual double getNumberParameter(const std::string& name) = 0;
    virtual const std::string getStringParameter(const std::string& name) = 0;
};

}

#endif
