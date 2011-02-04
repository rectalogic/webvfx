// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef Q_WS_MAC
#include <pthread.h>
#endif
#include <string.h>
#include <QApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include "webfx/webfx.h"
#include "webfx/web_logger.h"
#include "webfx/web_renderer.h"



namespace WebFX
{

static bool initialized = false;

static WebLogger* logger = 0;
static bool ownApp = false;

static QMutex initializeMutex;

#ifdef Q_WS_MAC
bool isMainThread();
#else
void* uiEventLoop(void*);
static pthread_t uiThread;
typedef QPair<QMutex*, QWaitCondition*> ThreadSync;
#endif

}


#ifndef Q_WS_MAC
void* WebFX::uiEventLoop(void* data)
{
    ThreadSync* threadSync = static_cast<ThreadSync*>(data);

    static const char *const empty = "";
    int argc = 1;
    QApplication app(argc, (char**)&empty);
    WebFX::ownApp = true;

    // Signal initialize() that app has been created
    {
        QMutexLocker eventLoopLock(threadSync->first);
        threadSync->second->wakeOne();
        threadSync = 0;
    }

    // Enter event loop
    app.exec();
    return 0;
}
#endif

void WebFX::setLogger(WebFX::WebLogger* logger)
{
    WebFX::logger = logger;
}

bool WebFX::initialize()
{
    QMutexLocker initLock(&WebFX::initializeMutex);

    if (WebFX::initialized)
        return true;

    // For non-mac, spawn a GUI application thread if qApp doesn't already exist.
    // For mac, the qApp must be created on the main thread, so check we are
    // on the main thread and create it - user must then call processEvents()
    // from the main thread.
    // http://bugreports.qt.nokia.com/browse/QTBUG-7393
    if (!qApp) {
#ifdef Q_WS_MAC
        if (!WebFX::isMainThread()) {
            WebFX::log("WebFX must be initialized on the main thread on MacOS");
            return false;
        }

        // Create a QApplication, we will delete it in processEvents()
        static const char *const empty = "";
        int argc = 1;
        new QApplication(argc, (char**)&empty);
        WebFX::ownApp = true;
#else
        {
            QMutex uiThreadMutex;
            QWaitCondition uiThreadCondition;
            ThreadSync uiThreadSync(&uiThreadMutex, &uiThreadCondition);

            QMutexLocker uiThreadLock(&uiThreadMutex);

            //XXX check return
            pthread_create(&WebFX::uiThread, 0, WebFX::uiEventLoop, &uiThreadSync);

            // Wait for signal that ui thread has created qApp
            uiThreadCondition.wait(&uiThreadMutex);
        }
#endif
    }

    // Register metatypes for queued connections
    qRegisterMetaType<WebFX::WebParameters*>("WebFX::WebParameters*");

    WebFX::initialized = true;
    return true;
}

WebFX::WebEffects* WebFX::createWebEffects()
{
    return new WebFX::WebRenderer();
}

int WebFX::processEvents()
{
#ifdef Q_WS_MAC
    // We didn't create the app, the user should be running its event loop.
    if (!WebFX::ownApp)
        return 0;
    if (!WebFX::isMainThread()) {
        WebFX::log("WebFX::processEvents() must be called from the main thread.");
        return 1;
    }
    int result = qApp->exec();
    delete qApp;
    return result;
#else
    return 0;
#endif
}

void WebFX::shutdown()
{
    QMutexLocker initLock(&WebFX::initializeMutex);

    if (!WebFX::initialized)
        return;

    // If we created uiThread, then we created QApplication.
    if (WebFX::ownApp) {
        // Quit the application
        if (qApp)
            qApp->quit();
#ifndef Q_WS_MAC
        // Wait for the app thread to finish
        pthread_join(WebFX::uiThread, 0);
#endif
        WebFX::ownApp = false;
    }
    //XXX if we don't own, should we post an event and wait for it to ensure any references to our stuff are off the event loop before our lib is unloaded?

    if (WebFX::logger) {
        delete WebFX::logger;
        WebFX::logger = 0;
    }
}

void WebFX::log(const std::string& msg)
{
    if (WebFX::logger)
        WebFX::logger->log(msg);
}
