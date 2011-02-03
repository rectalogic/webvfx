// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEBFX_H_
#define WEBFX_WEBFX_H_

#include <webfx/web_effects.h>
#include <webfx/web_logger.h>

namespace WebFX {

// initialize must only be called once.
// A WebLogger implementation may be supplied, it will be deleted at shutdown.
bool initialize(WebLogger* logger=0);

WebEffects* createWebEffects();

void shutdown();

}
#endif
