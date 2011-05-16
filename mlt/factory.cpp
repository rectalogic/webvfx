// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QString>
#include <webvfx/webvfx.h>
extern "C" {
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_log.h>
    #include <mlt/framework/mlt_repository.h>
}
#include "factory.h"


namespace MLTWebVfx
{
class Logger : public WebVfx::Logger
{
    void log(const QString& message) {
        //XXX use log level once passed
        //XXX any way to get service into here?
        mlt_log(NULL, MLT_LOG_INFO, "%s\n", message.toLatin1().constData());
    }
};
}

static void* createService(mlt_profile profile,
                           mlt_service_type serviceType,
                           const char*, const void* fileName)
{
    if (!WebVfx::initialize())
        return 0;

    mlt_service service = 0;
    switch (serviceType) {
        case producer_type:
            service = MLTWebVfx::createProducer(profile);
            break;
        case filter_type:
            service = MLTWebVfx::createFilter();
            break;
        case transition_type:
            service = MLTWebVfx::createTransition();
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
#define EXPORT __attribute__((visibility ("default")))
#else
#define EXPORT
#endif

extern "C" EXPORT MLT_REPOSITORY
{
    MLT_REGISTER(producer_type, "webvfx", createService);
    MLT_REGISTER(filter_type, "webvfx", createService);
    MLT_REGISTER(transition_type, "webvfx", createService);

    // Register shutdown hook - even if we don't initialize WebVfx
    // we want our logger deleted.
    mlt_factory_register_for_clean_up(0, reinterpret_cast<mlt_destructor>(WebVfx::shutdown));
    WebVfx::setLogger(new MLTWebVfx::Logger());
}
