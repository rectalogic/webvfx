// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_PARAMETERS_H_
#define WEBVFX_PARAMETERS_H_

#include <string>

namespace WebVfx
{

class Parameters
{
public:
    Parameters() {};
    virtual ~Parameters() = 0;
    virtual double getNumberParameter(const std::string& name);
    virtual std::string getStringParameter(const std::string& name);
};

}

#endif
