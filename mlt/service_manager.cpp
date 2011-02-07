// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <webfx/webfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
}
#include "effects_manager.h"


namespace MLTWebFX
{
const char* ServiceManager::kWebFXPropertyName = "WebFX";
const char* ServiceManager::kURLPropertyName = "WebFXURL";

class ServiceParameters : public WebFX::WebParameters
{
public:
    ServiceParameters(mlt_service service)
        : properties(MLT_SERVICE_PROPERTIES(service)) {
    }

    double getNumberParameter(const std::string& name) {
        return mlt_properties_get_double(properties, name.c_str());
    }

    const std::string getStringParameter(const std::string& name) {
        return mlt_properties_get(properties, name.c_str());
    }

private:
    mlt_properties properties;
};


ServiceManager::ServiceManager(mlt_service service)
    : service(service)
{
    mlt_service_lock(service);
}

ServiceManager::~ServiceManager()
{
    mlt_service_unlock(service);
}

bool ServiceManager::initialize(int width, int height)
{
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);
    webEffects = (WebFX::WebEffects*)mlt_properties_get_data(properties, ServiceManager::kWebFXPropertyName, 0);
    if (!webEffects) {
        const char* url = mlt_properties_get(properties, ServiceManager::kURLPropertyName);
        if (!url) {
            mlt_log(service, MLT_LOG_ERROR, "No %s property found\n", ServiceManager::kURLPropertyName);
            return false;
        }
        webEffects = WebFX::createWebEffects();
        bool result = webEffects->initialize(url, width, height, new ServiceParameters(service));
        if (!result) {
            delete webEffects; webEffects = 0;
            mlt_log(service, MLT_LOG_ERROR, "Failed to create WebEffects\n");
            return result;
        }
        //XXX process ImageTypeMap - setup producers and save src/dst names

        mlt_properties_set_data(properties, ServiceManager::kWebFXPropertyName, webEffects, 0, (mlt_destructor)destroyWebEffects, NULL);
    }
    return true;
}

WebFX::WebEffects* ServiceManager::getWebEffects()
{
    return webEffects;
}

void ServiceManager::destroyWebEffects(WebFX::WebEffects* webEffects)
{
    webEffects->destroy();
}

}
