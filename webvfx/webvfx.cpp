// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef Q_WS_MAC
#include <pthread.h>
#endif
#include <cstdlib>
#include <QApplication>
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

#ifdef Q_WS_MAC
bool isMainThread();
#else
static pthread_t s_uiThread;
typedef QPair<QMutex*, QWaitCondition*> ThreadSync;

void* uiEventLoop(void* data)
{
    ThreadSync* threadSync = static_cast<ThreadSync*>(data);

    static const char *const empty = "";
    int argc = 1;
    QApplication app(argc, (char**)&empty);
    s_ownApp = true;

    // Signal s_initialized() that app has been created
    {
        QMutexLocker eventLoopLock(threadSync->first);
        threadSync->second->wakeOne();
        threadSync = 0;
    }

    // Enter event loop
    app.exec();
    QCoreApplication::processEvents();
    return 0;
}
#endif

void setLogger(Logger* logger)
{
    s_logger = logger;
}

bool initialize()
{
    QMutexLocker initLock(&s_initializedMutex);

    if (s_initialized)
        return true;

    // For non-mac, spawn a GUI application thread if qApp doesn't already exist.
    // For mac, the qApp must be created on the main thread, so check we are
    // on the main thread and create it - user must then call processEvents()
    // from the main thread.
    // http://bugreports.qt.nokia.com/browse/QTBUG-7393
    if (!qApp) {
#ifdef Q_WS_MAC
        if (!isMainThread()) {
            log("WebVfx must be initialized on the main thread on MacOS");
            return false;
        }

        // Create a QApplication, we will delete it in processEvents()
        static const char *const empty = "";
        int argc = 1;
        new QApplication(argc, (char**)&empty);
        s_ownApp = true;
#else
        {
#ifdef Q_WS_X11
            if (std::getenv("DISPLAY") == 0) {
                log("DISPLAY environment variable not set");
                return false;
            }
#endif
            QMutex uiThreadMutex;
            QWaitCondition uiThreadCondition;
            ThreadSync uiThreadSync(&uiThreadMutex, &uiThreadCondition);

            QMutexLocker uiThreadLock(&uiThreadMutex);

            //XXX check return
            pthread_create(&s_uiThread, 0, uiEventLoop, &uiThreadSync);

            // Wait for signal that ui thread has created qApp
            uiThreadCondition.wait(&uiThreadMutex);
        }
#endif
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

int processEvents()
{
#ifdef Q_WS_MAC
    // We didn't create the app, the user should be running its event loop.
    if (!s_ownApp)
        return 0;
    if (!isMainThread()) {
        log("WebVfx::processEvents() must be called from the main thread.");
        return 1;
    }
    int result = qApp->exec();
    delete qApp;
    return result;
#else
    return 0;
#endif
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
#ifndef Q_WS_MAC
        // Wait for the app thread to finish
        pthread_join(s_uiThread, 0);
#endif
        s_ownApp = false;
    }
    else
        QCoreApplication::processEvents();
}

void log(const QString& msg)
{
    if (s_logger)
        s_logger->log(msg);
}

}
