// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEBFX_H_
#define WEBFX_WEBFX_H_

#include <webfx/web_effects.h>
#include <webfx/web_logger.h>

namespace WebFX {

// WebFX will take ownership of the logger and delete at shutdown.
// setLogger must be called only once and before initialize().
void setLogger(WebLogger* logger);

// initialize is threadsafe and may be called multiple times,
// On MacOS, in a non Qt host application, initialize and processEvents
// must be called from the main thread.
// In a Qt based MacOS application, or a non-MacOS application,
// initialize can be called from any thread and processEvents need not be called.
// http://bugreports.qt.nokia.com/browse/QTBUG-7393
bool initialize();

// Create a WebEffects instance.
// WebEffects::initialize() should be called to initialize the new instance.
WebEffects* createWebEffects();

// Must be called after initialize from the main thread in
// non-Qt based MacOS applications if WebEffects is going to be used
// from any other threads.
int processEvents();

void shutdown();

}
#endif
