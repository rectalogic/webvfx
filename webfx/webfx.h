// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEBFX_H_
#define WEBFX_WEBFX_H_

#include <webfx/web_effects.h>
#include <webfx/logger.h>

namespace WebFX {

// initialize must only be called once.
// argv must be valid until shutdown() is called.
// argc must be > 0 and argv must have at least an empty string.
// argv may be modified.
// A Logger implementation may be supplied, it will be deleted at shutdown.
bool initialize(int argc, char* argv[], Logger* logger=0);

WebEffects* createWebEffects();

void shutdown();

}
#endif
