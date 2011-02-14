// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QString>
#include <webvfx/webvfx.h>
extern "C" {
    #include <mlt/framework/mlt_log.h>
    #include <mlt/framework/mlt_repository.h>
}
#include "webvfx_service.h"


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

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__((visibility ("default")))
#else
#define EXPORT
#endif

extern "C" EXPORT MLT_REPOSITORY
{
    MLTWebVfx::registerServices(repository, producer_type);
    MLTWebVfx::registerServices(repository, filter_type);
    MLTWebVfx::registerServices(repository, transition_type);

    // Register shutdown hook - even if we don't initialize WebVfx
    // we want our logger deleted.
    mlt_factory_register_for_clean_up(0, reinterpret_cast<mlt_destructor>(WebVfx::shutdown));
    WebVfx::setLogger(new MLTWebVfx::Logger());
}
