// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
#include <framework/mlt_factory.h>
#include <framework/mlt_log.h>
#include <framework/mlt_repository.h>
}
#include "factory.h"

namespace WebVfxPlugin {

static void* createService(mlt_profile profile,
    mlt_service_type serviceType,
    const char*, const void* fileName)
{
    mlt_service service = 0;
    switch (serviceType) {
    case mlt_service_producer_type:
        service = WebVfxPlugin::createProducer(profile);
        break;
    case mlt_service_filter_type:
        service = WebVfxPlugin::createFilter();
        break;
    case mlt_service_transition_type:
        service = WebVfxPlugin::createTransition();
        break;
    default:
        return 0;
        break;
    }

    if (fileName) {
        mlt_properties_set(MLT_SERVICE_PROPERTIES(service),
            "resource", static_cast<const char*>(fileName));
    }

    return service;
}

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

extern "C" EXPORT MLT_REPOSITORY
{
    MLT_REGISTER(mlt_service_producer_type, "mltwebvfx", createService);
    MLT_REGISTER(mlt_service_filter_type, "mltwebvfx", createService);
    MLT_REGISTER(mlt_service_transition_type, "mltwebvfx", createService);
}

}