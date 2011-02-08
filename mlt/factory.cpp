// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <webvfx/webvfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
    #include <mlt/framework/mlt_repository.h>
}
#include "webvfx_service.h"


namespace MLTWebVFX
{
class Logger : public WebVFX::WebLogger
{
    void log(const std::string& message) {
        //XXX use log level once passed
        //XXX any way to get service into here?
        mlt_log(NULL, MLT_LOG_INFO, "%s\n", message.c_str());
    }
};
}

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__((visibility ("default")))
#else
#define EXPORT
#endif

extern "C" EXPORT MLT_REPOSITORY
{
    MLTWebVFX::registerServices(repository, producer_type);
    MLTWebVFX::registerServices(repository, filter_type);
    MLTWebVFX::registerServices(repository, transition_type);

    // Register shutdown hook - even if we don't initialize WebVFX
    // we want our logger deleted.
    mlt_factory_register_for_clean_up(0, WebVFX::shutdown);
    WebVFX::setLogger(new MLTWebVFX::Logger());
}
