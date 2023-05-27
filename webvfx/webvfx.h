// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "parameters.h"

/*!
 * @brief Public entry points into WebVfx
 *
 * These are the methods and classes that will be needed to host
 * WebVfx effect rendering.
 */
namespace WebVfx {

/*!
 * @brief Initialize the WebVfx framework
 *
 * initialize() is threadsafe and may be called multiple times.
 *
 * On MacOS, in a non Qt host application, initialize() and processEvents()
 * must be called from the main thread.
 * In a Qt based MacOS application, or a non-MacOS application,
 * initialize() can be called from any thread and processEvents()
 * need not be called.
 * See <a href="http://bugreports.qt.nokia.com/browse/QTBUG-7393">QTBUG-7393</a>.
 *
 * @return Indicates whether initialization was successful
 */
bool initialize();

}
