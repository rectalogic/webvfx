// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

extern "C" {
#include <framework/mlt_service.h> // for mlt_service_s
#include <framework/mlt_types.h> // for mlt_service, mlt_position
}
namespace WebVfxPlugin {
class ServiceManager;
}

namespace WebVfxPlugin {

class ServiceLocker {
public:
    ServiceLocker(mlt_service service);
    ~ServiceLocker();

    bool initialize(mlt_position length);
    ServiceManager* getManager();

private:
    static const char* kManagerPropertyName;
    static void destroyManager(ServiceManager* manager);
    mlt_service service;
    ServiceManager* manager;
};

}