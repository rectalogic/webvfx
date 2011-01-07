// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTCHROMIX_CHROMIX_HELPER_H_
#define MLTCHROMIX_CHROMIX_HELPER_H_

extern "C" {
    #include <mlt/framework/mlt_factory.h>
}
#include <string>

#define CHROMIX_METADATA_PROP "ChromixMetadata"

void chromix_register_services(mlt_repository repository, mlt_service_type service_type);

const std::string chromix_get_metadata_dir();

#endif
