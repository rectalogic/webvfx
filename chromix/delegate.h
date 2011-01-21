// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_DELEGATE_H_
#define CHROMIX_DELEGATE_H_

#include <string>
#include <v8/include/v8.h>

namespace Chromix {

class Delegate {
public:
    Delegate() {}
    virtual ~Delegate() {}

    virtual void logMessage(const std::string& message) = 0;

    virtual v8::Handle<v8::Value> getParameterValue(const std::string& name) = 0;

    v8::Handle<v8::Value> getUndefinedParameterValue();
    v8::Handle<v8::Value> wrapParameterValue(bool value);
    v8::Handle<v8::Value> wrapParameterValue(double value);
    v8::Handle<v8::Value> wrapParameterValue(const std::string& value);
};

}

#endif
