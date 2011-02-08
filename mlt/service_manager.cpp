// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <webvfx/webvfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
}
#include "effects_manager.h"


namespace MLTWebVFX
{
const char* ServiceManager::kURLPropertyName = "WebVFXURL";

class ServiceParameters : public WebVFX::WebParameters
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
}

ServiceManager::~ServiceManager()
{
    if (webEffects)
        webEffects->destroy();
}

bool ServiceManager::initialize(int width, int height)
{
    if (webEffects)
        return true;

    const char* url = mlt_properties_get(properties, ServiceManager::kURLPropertyName);
    if (!url) {
        mlt_log(service, MLT_LOG_ERROR, "No %s property found\n", ServiceManager::kURLPropertyName);
        return false;
    }
    webEffects = WebVFX::createWebEffects();
    bool result = webEffects->initialize(url, width, height, new ServiceParameters(service));
    if (!result) {
        delete webEffects; webEffects = 0;
        mlt_log(service, MLT_LOG_ERROR, "Failed to create WebEffects\n");
        return result;
    }
    //XXX process ImageTypeMap - setup producers and save src/dst names

    return true;
}

WebVFX::WebEffects* ServiceManager::getWebEffects()
{
    return webEffects;
}

}
