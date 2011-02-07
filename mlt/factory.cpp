// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <webfx/webfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
    #include <mlt/framework/mlt_repository.h>
}
#include "webfx_service.h"


class Logger : public WebFX::WebLogger
{
    void log(const std::string& message) {
        //XXX use log level once passed
        //XXX any way to get service into here?
        mlt_log(NULL, MLT_LOG_INFO, "%s\n", message.c_str());
    }
};

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__((visibility ("default")))
#else
#define EXPORT
#endif

extern "C" EXPORT MLT_REPOSITORY
{
    webfx_register_services(repository, producer_type);
    webfx_register_services(repository, filter_type);
    webfx_register_services(repository, transition_type);

    // Register shutdown hook - even if we don't initialize WebFX
    // we want our logger deleted.
    mlt_factory_register_for_clean_up(NULL, WebFX::shutdown);
    WebFX::setLogger(new Logger());
}
