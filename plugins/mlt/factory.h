// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

extern "C" {
#include <framework/mlt_types.h> // for mlt_service, mlt_profile
}

namespace WebVfxPlugin {

mlt_service createProducer(mlt_profile profile);
mlt_service createFilter();
mlt_service createTransition();

}
