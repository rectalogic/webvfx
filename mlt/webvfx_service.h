// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBVFX_WEBVFX_SERVICE_H_
#define MLTWEBVFX_WEBVFX_SERVICE_H_

extern "C" {
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_service.h>
}
#include <string>

namespace MLTWebVfx
{

void registerServices(mlt_repository repository, mlt_service_type serviceType);

mlt_service createProducer();
mlt_service createFilter();
mlt_service createTransition();

}

#endif
