// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webvfx.h"
#include <QMutex> // for QMutex
#include <QMutexLocker> // for QMutexLocker

namespace WebVfx {

static bool s_initialized = false;
static QMutex s_initializedMutex;

bool initialize()
{
    QMutexLocker initLock(&s_initializedMutex);

    if (s_initialized)
        return true;

    s_initialized = true;
    return true;
}

}
