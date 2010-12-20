// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix_task.h"
#include "chromix_context.h"
#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
extern "C" {
#include <mlt_log.h>
}

static mlt_deque ChromixTask::queue = 0;
static pthread_t ChromixTask::chromixThread = 0;
static pthread_mutex_t ChromixTask::queueMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ChromixTask::queueCond = PTHREAD_COND_INITIALIZER;

static const char* TASK_PROP = "ChromixTask";

////////////////////////////////

class AutoChromix {
public:
    //XXX check return code and propagate somehow
    AutoChromix(int argc, const char * argv[]) { Chromix::initialize(argc, argv); }
    ~AutoChromix() { Chromix::shutdown(); }
};

////////////////////////////////

class MutexLock {
public:
    MutexLock(pthread_mutex_t* mutex) : mutex(mutex) { pthread_mutex_lock(mutex); }
    ~MutexLock() { pthread_mutex_unlock(mutex); }
private:
    pthread_mutex_t *mutex;
};

////////////////////////////////

ChromixTask* ChromixTask::getTask(mlt_service service);
    return (ChromixTask*)mlt_properties_get_data(MLT_SERVICE_PROPERTIES(service), TASK_PROP, NULL);
}

void ChromixTask::chromixMainLoop(void* arg) {
    AutoChromix chromix(argc, argv);

    //XXX check shutdown flag in this loop, but also drain queue
    pthread_mutex_lock(&queueMutex);
    while (true) {
        // This manages autorelease pool for Mac
        ChromixContext context;

        //XXX need to check shutdown flag in this loop too
        while ((ChromixTask* task = nextQueuedTask()) == 0)
            pthread_cond_wait(&queueCond);
        task->executeTask();
    }
    pthread_mutex_unlock(&queueMutex);

    pthread_cond_destroy(&queueCond);
    pthread_mutex_destroy(&queueMutex);
    mlt_queue_close(queue);
    //XXX should all this be in AutoChromix? do we even need AutoChromix?
}

void ChromixTask::shutdownHook(void*) {
//XXX set flag to drain queue and exit chromix thread
//XXX join on the chromix thread
}

// mlt_destructor for service task property.
// Flag task for destruction, queue up a task then delete it when finished.
void ChromixTask::destroy(ChromixTask *task) {
    task->needsDestruction = true;
    task->queueAndWait();
    delete task;
}

////////////////////////////////

ChromixTask::ChromixTask(mlt_service service) :
    service(service),
    mixRender(0),
    taskCond(PTHREAD_COND_INITIALIZER),
    needsDestruction(false),
    taskResult(0)
{
    mlt_properties_set_data(MLT_SERVICE_PROPERTIES(service), TASK_PROP, this, 0,
                            (mlt_destructor)destroy, NULL);
}

ChromixTask::~ChromixTask() {
    //XXX not safe to destroy while other threads are waiting
    pthread_cond_destroy(&taskCond);
    //XXX assert that mixRender is NULL, it should have been destroyed on chromix thread
}

int ChromixTask::renderToImageForTime(ChromixRawImage targetImage, double time) {
    this.targetImage = targetImage;
    this.time = time;
    return queueAndWait();
}

int ChromixTask::queueAndWait() {
    MutexLock lock(&queueMutex);
    taskResult = 0;

    // Create queue and thread lazily
    if (chromixThread == 0) {
        queue = mlt_deque_init();
        if (pthread_create(&chromixThread, NULL, chromixMainLoop, 0) < 0) {
            mlt_log(NULL, MLT_LOG_FATAL, "failed to spawn Chromix thread");
            return 1;
        }
        mlt_factory_register_for_clean_up(NULL, shutdownHook);
    }

    // Add ourself to queue and signal
    mlt_deque_push_back(queue, this);
    pthread_cond_signal(&queueCond);

    // Wait for task to execute
    pthread_cond_wait(&taskCond, &queueMutex);

    return taskResult;
}

int ChromixTask::initMixRender() {
    if (mixRender)
        return 0;
    mixRender = new Chromix::MixRender();
    if (!mixRender) {
        mlt_log(service, MLT_LOG_ERROR, "failed to create MixRender");
        return 1;
    }
    //XXX load url
    //XXX need to set mixRender properties from mlt_properties_get, iterate over metadata
    //XXX should chromix use a callback to get properties from the app? would need to map to v8 datatype
    mixRender->loadURL(WTF::String::fromUTF8("file://localhost/Users/aw/Projects/snapfish/encoder/chromix/test/test.html"));//XXX
    //XXX get and load html page property here, if not specified assume same as service_name".html"

    //XXX for a_track/b_track for transition, "track" for filter, and other tracks
    //XXX add these in yaml as custom props - mapping track to prop name used in html?
    //XXX store this mapping on properties - map char* well defined name (for filter, trans) to WTF::String we can use w/chromix

    //XXX add method dealing with getting the writeable image etc.

    //XXX setup Chromix log callback using mlt_log - should we map chrome levels to mlt? are crhome levels accurate?
    return 0;
}

int setImageForName(ChromixRawImage image, WTF::String name) {
    //XXX assert we are on chromix thread
    unsigned char* buffer = mixRender->writeableDataForImageParameter(name, image.width, image.height);
    if (!buffer)
        return 1;
    memcpy(buffer, image.image, image.width * image.height * 4);
    return 0;
}

int renderToTarget() {
    // Resize MixRender and render into target image
    mixRender->resize(targetImage.width, targetImage.height);
    //XXX move this into MixRender - pass raw buffer for it to render into
    const SkBitmap* skiaBitmap = mixRender->render(time);
    if (!skiaBitmap)
        return 1;
    SkAutoLockPixels bitmapLock(*skiaBitmap);
    // Do a block copy if no padding, otherwise copy a row at a time
    unsigned int byteCount = targetImage.width * targetImage.height * 4;
    if (skiaBitmap->getSize() == byteCount)
        memcpy(targetImage.image, skiaBitmap->getPixels(), byteCount);
    else {
        int bytesPerRow = targetImage.width * 4;
        const unsigned char* srcP = reinterpret_cast<const unsigned char*>(skiaBitmap->getPixels());
        unsigned char* dstP = targetImage.image;
        for (int y = 0; y < targetImage.height; y++) {
            memcpy(dstP, srcP, bytesPerRow);
            srcP += skiaBitmap->rowBytes();
            dstP += bytesPerRow;
        }
    }
    targetImage.reset();
    return 0;
}

void ChromixTask::executeTask() {
    if (needsDestruction) {
        // Safe to destroy MixRender on this thread.
        delete mixRender; mixRender = 0;
        pthread_cond_signal(&taskCond);
    }
    else {
        // Lazily perform one-time initialization of MixRender
        taskResult = initMixRender();
        // Allow subclass to process
        if (taskResult == 0) {
            taskResult = performTask();
            if (taskResult == 0)
                taskResult = renderToTarget();
        }
        pthread_cond_signal(&taskCond);
    }
}


//XXX custom C++ classes for filter/producer/trans etc. which can hold needed data and handle the rendering


