// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFXPIPE_FACTORY_H_
#define VFXPIPE_FACTORY_H_

extern "C" {
#include <framework/mlt_profile.h>
#include <framework/mlt_service.h>
}

namespace VFXPipe {

mlt_service createProducer(mlt_profile profile);
mlt_service createFilter();
mlt_service createTransition();

}

#endif
