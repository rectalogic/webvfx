// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include "webvfx/logger.h"
#include "webvfx/webvfx.h"

namespace WebVfx
{

static bool s_initialized = false;
static Logger* s_logger = 0;
static QMutex s_initializedMutex;


void setLogger(Logger* logger)
{
    s_logger = logger;
}

bool initialize()
{
    QMutexLocker initLock(&s_initializedMutex);

    if (s_initialized)
        return true;

    s_initialized = true;
    return true;
}

void log(const QString& msg)
{
    if (s_logger)
        s_logger->log(msg);
}

}
