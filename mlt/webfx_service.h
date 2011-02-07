// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBFX_WEBFX_SERVICE_H_
#define MLTWEBFX_WEBFX_SERVICE_H_

extern "C" {
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_types.h>
}
#include <string>

void webfx_register_services(mlt_repository repository, mlt_service_type service_type);

const std::string webfx_get_metadata_dir();
mlt_properties chromix_load_metadata(const std::string& service_name);

mlt_producer webfx_producer_create(const char* service_name);
mlt_filter webfx_filter_create(const char* service_name);
mlt_producer webfx_transition_create(const char* service_name);

#endif
