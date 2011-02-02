// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <pthread.h>
#include <string.h>
#include <QApplication>
#include "webfx/webfx.h"
#include "webfx/web_logger.h"
#include "webfx/web_renderer.h"



namespace WebFX
{
typedef struct {
    int argc;
    char** argv;
} Args;

void* uiEventLoop(void*);

static bool ownApp = false;
static pthread_t uiThread;
static pthread_mutex_t uiMutex;
static pthread_cond_t uiCond;

static WebFX::WebLogger* logger = 0;
}



void* WebFX::uiEventLoop(void* args)
{
    int argc = static_cast<WebFX::Args*>(args)->argc;
    char** argv = static_cast<WebFX::Args*>(args)->argv;

    QApplication app(argc, argv);
    WebFX::ownApp = true;

    // Signal initialize() that app has been created
    pthread_mutex_lock(&WebFX::uiMutex);
    pthread_cond_signal(&WebFX::uiCond);
    pthread_mutex_unlock(&WebFX::uiMutex);

    // Enter event loop
    app.exec();
    return 0;
}

bool WebFX::initialize(int argc, char* argv[], WebFX::WebLogger* logger)
{
    WebFX::logger = logger;

    // Spawn GUI application thread if qApp doesn't already exist
    if (!qApp) {
        WebFX::Args args;
        args.argc = argc;
        args.argv = argv;

        //XXX check return values from all these
        pthread_mutex_init(&WebFX::uiMutex, 0);
        pthread_cond_init(&WebFX::uiCond, 0);
        pthread_mutex_lock(&WebFX::uiMutex);

        pthread_create(&WebFX::uiThread, 0, WebFX::uiEventLoop, &args);

        // Wait for signal that app has been created
        pthread_cond_wait(&WebFX::uiCond, &WebFX::uiMutex);

        pthread_mutex_unlock(&WebFX::uiMutex);
        pthread_cond_destroy(&WebFX::uiCond);
        pthread_mutex_destroy(&WebFX::uiMutex);
    }
    return true;
}

WebFX::WebEffects* WebFX::createWebEffects()
{
    return new WebFX::WebRenderer();
}

void WebFX::shutdown()
{
    // If we created uiThread, then we created QApplication.
    if (WebFX::ownApp) {
        // Quit the application and wait for the thread to finish.
        if (qApp)
            qApp->quit();
        pthread_join(WebFX::uiThread, 0);
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
