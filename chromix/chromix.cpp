// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <pthread.h>
#include <string.h>
#include <QApplication>
#include "chromix/chromix.h"
#include "chromix/web_renderer.h"
#include "chromix/logger.h"



namespace Chromix
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

static Chromix::Logger* logger = 0;
}



void* Chromix::uiEventLoop(void* args)
{
    int argc = static_cast<Chromix::Args*>(args)->argc;
    char** argv = static_cast<Chromix::Args*>(args)->argv;

    QApplication app(argc, argv);
    Chromix::ownApp = true;

    // Signal initialize() that app has been created
    pthread_mutex_lock(&Chromix::uiMutex);
    pthread_cond_signal(&Chromix::uiCond);
    pthread_mutex_unlock(&Chromix::uiMutex);

    // Enter event loop
    app.exec();
}

bool Chromix::initialize(int argc, char* argv[], Chromix::Logger* logger)
{
    Chromix::logger = logger;

    // Spawn GUI application thread if qApp doesn't already exist
    if (!qApp) {
        Chromix::Args args;
        args.argc = argc;
        args.argv = argv;

        //XXX check return values from all these
        pthread_mutex_init(&Chromix::uiMutex, 0);
        pthread_cond_init(&Chromix::uiCond, 0);
        pthread_mutex_lock(&Chromix::uiMutex);

        pthread_create(&Chromix::uiThread, 0, Chromix::uiEventLoop, &args);

        // Wait for signal that app has been created
        pthread_cond_wait(&Chromix::uiCond, &Chromix::uiMutex);

        pthread_mutex_unlock(&Chromix::uiMutex);
        pthread_cond_destroy(&Chromix::uiCond);
        pthread_mutex_destroy(&Chromix::uiMutex);
    }
    return true;
}

Chromix::MixKit* Chromix::createMixKit()
{
    return new Chromix::WebRenderer();
}

void Chromix::shutdown()
{
    // If we created uiThread, then we created QApplication.
    if (Chromix::ownApp) {
        // Quit the application and wait for the thread to finish.
        if (qApp)
            qApp->quit();
        pthread_join(Chromix::uiThread, 0);
        Chromix::ownApp = false;
    }

    if (Chromix::logger) {
        delete Chromix::logger;
        Chromix::logger = 0;
    }
}

void Chromix::log(const std::string& msg)
{
    if (Chromix::logger)
        Chromix::logger->log(msg);
}
