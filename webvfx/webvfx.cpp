// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdlib>
#include <QAnimationDriver>
#include <QGuiApplication>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QPair>
#include <QString>
#include <QWaitCondition>
#include "webvfx/logger.h"
#include "webvfx/webvfx.h"
#include "webvfx/effects_impl.h"

namespace WebVfx
{

static bool s_initialized = false;

static Logger* s_logger = 0;
static bool s_ownApp = false;

static QMutex s_initializedMutex;

static pthread_t s_uiThread;
typedef QPair<QMutex*, QWaitCondition*> ThreadSync;

void* uiEventLoop(void* data)
{
    ThreadSync* threadSync = static_cast<ThreadSync*>(data);
    // Can't use offscreen with linux
    // https://bugreports.qt.io/browse/QTBUG-90992
    const char * argv[] =
    #ifdef Q_OS_MACOS
        {"", "-platform", "offscreen"};
    #else
        {""};
    #endif
    int argc = sizeof(argv) / sizeof(argv[0]);
    QGuiApplication app(argc, (char **)argv);
    s_ownApp = true;

    // Signal s_initialized() that app has been created
    {
        QMutexLocker eventLoopLock(threadSync->first);
        threadSync->second->wakeOne();
        threadSync = 0;
    }

    // Enter event loop
    app.exec();
    QGuiApplication::processEvents();

    return 0;
}

void setLogger(Logger* logger)
{
    s_logger = logger;
}

bool initialize()
{
    QMutexLocker initLock(&s_initializedMutex);

    if (s_initialized)
        return true;

    // Spawn a GUI application thread if qApp doesn't already exist.
    if (!qApp) {
        {
            QMutex uiThreadMutex;
            QWaitCondition uiThreadCondition;
            ThreadSync uiThreadSync(&uiThreadMutex, &uiThreadCondition);

            QMutexLocker uiThreadLock(&uiThreadMutex);

            //XXX check return
            pthread_create(&s_uiThread, 0, uiEventLoop, &uiThreadSync);

            // Wait for signal that ui thread has created qApp
            uiThreadCondition.wait(&uiThreadMutex);
        }
    }

    // Register metatypes for queued connections
    qRegisterMetaType<Parameters*>("Parameters*");
    qRegisterMetaType<Image*>("Image*");

    s_initialized = true;
    return true;
}

Effects* createEffects(const QString& fileName, int width, int height, Parameters* parameters)
{
    EffectsImpl* effects = new EffectsImpl();
    if (!effects->initialize(fileName, width, height, parameters)) {
        effects->destroy();
        return 0;
    }
    return effects;
}

void shutdown()
{
    QMutexLocker initLock(&s_initializedMutex);

    // Delete the s_logger even if not initialized
    delete s_logger; s_logger = 0;

    if (!s_initialized)
        return;

    // If we created s_uiThread, then we created QApplication.
    if (s_ownApp) {
        // Quit the application
        if (qApp)
            qApp->quit();
        // Wait for the app thread to finish
        pthread_join(s_uiThread, 0);
        s_ownApp = false;
    }
    else
        QGuiApplication::processEvents();
}

void log(const QString& msg)
{
    if (s_logger)
        s_logger->log(msg);
}

}
