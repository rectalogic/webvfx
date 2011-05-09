// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBVFX_FACTORY_H_
#define MLTWEBVFX_FACTORY_H_

extern "C" {
    #include <mlt/framework/mlt_service.h>
    #include <mlt/framework/mlt_profile.h>
}

namespace MLTWebVfx
{

mlt_service createProducer(mlt_profile profile);
mlt_service createFilter();
mlt_service createTransition();

}

#endif
