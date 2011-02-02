// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_PARAMETERS_H_
#define CHROMIX_PARAMETERS_H_

#include <string>

namespace Chromix
{

class Parameters
{
public:
    virtual double getNumberParameter(const std::string& name) { return 0; };
    virtual const std::string getStringParameter(const std::string& name) { return std::string(); };

protected:
    Parameters() {};
    virtual ~Parameters() = 0;
};

}

#endif
