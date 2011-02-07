// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBFX_WEBFX_SERVICE_H_
#define MLTWEBFX_WEBFX_SERVICE_H_

extern "C" {
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_service.h>
}
#include <string>

namespace MLTWebFX
{

void registerServices(mlt_repository repository, mlt_service_type serviceType);

mlt_service createProducer(const char* serviceName);
mlt_service createFilter(const char* serviceName);
mlt_service createTransition(const char* serviceName);

}

#endif
