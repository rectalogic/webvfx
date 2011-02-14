// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <webvfx/webvfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
}
#include "service_locker.h"
#include "service_manager.h"


namespace MLTWebVfx
{
const char* ServiceLocker::kManagerPropertyName = "WebVfxManager";


ServiceLocker::ServiceLocker(mlt_service service)
    : service(service)
{
    mlt_service_lock(service);
}

ServiceLocker::~ServiceLocker()
{
    mlt_service_unlock(service);
}

bool ServiceLocker::initialize(int width, int height)
{
    // If we don't have a ServiceManager, create one and store on service
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);
    manager = static_cast<ServiceManager*>(mlt_properties_get_data(properties, ServiceLocker::kManagerPropertyName, 0));
    if (!manager) {
        manager = new ServiceManager(service);
        bool result = manager->initialize(width, height);
        if (!result) {
            destroyManager(manager);
            mlt_log(service, MLT_LOG_ERROR, "Failed to create WebVfx ServiceManager\n");
            return result;
        }

        mlt_properties_set_data(properties, kManagerPropertyName, manager, 0, reinterpret_cast<mlt_destructor>(destroyManager), NULL);
    }
    return true;
}

ServiceManager* ServiceLocker::getManager()
{
    return manager;
}

void ServiceLocker::destroyManager(ServiceManager* manager)
{
    delete manager;
}

}
