// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <webfx/webfx.h>
#include "effects_manager.h"


const char* ServiceManager::kWebFXPropertyName = "WebFX";

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

bool ServiceManager::initialize(const char* url, int width, int height)
{
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);
    webEffects = (WebFX::WebEffects*)mlt_properties_get_data(properties, ServiceManager::kWebFXPropertyName, NULL);
    if (!webEffects) {
        webEffects = WebFX::createWebEffects();
        bool result = webEffects->initialize(url, width, height, new ServiceParameters(service));
        if (!result) {
            delete webEffects; webEffects = 0;
            return result;
        }
        //XXX need to initialize, check return - so need html and size
        //XXX need parameters delegate
        //XXX need to know src/dst/extra images so caller can get them
        //XXX should handle extra producers in this class
        //
        //XXX should we bail on YAML and use webfx.filter.name.html etc.
        //XXX so register each of these, and query it to get metadata on demand
        //XXX and get video names and other param data via html metadata api
        //XXX when we create the mlt_service we know it's name and should stash the URL on a property
        //
        //XXX just need array of names and types - types can be const Q_PROPERTYs (FINAL CONSTANT)
        //XXX e.g. webfx.SRC_VIDEO, webfx.NUMBER etc.
        //XXX or hash of names to types, passed back to loadFinished
        //XXX or should we fire signal into page - requestSrcVideo etc.? hmm signals don't return values
        //
        //XXX also allow yaml - if we find one, use it for metadata - and add our video info?
        //
        //XXX can we make user specify props? a_video=srcVideo etc.?
        //XXX or enumerate video.* properties - which specify src/dst/extra
        mlt_properties_set_data(properties, ServiceManager::kWebFXPropertyName, webEffects, 0, (mlt_destructor)destroyWebEffects, NULL);
    }
}

WebFX::WebEffects* ServiceManager::getWebEffects()
{
    return webEffects;
}

void ServiceManager::destroyWebEffects(WebFX::WebEffects* webEffects)
{
    webEffects->destroy();
}
