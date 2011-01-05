// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix_task.h"
#include "chromix_context.h"
#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
#include <chromix/Delegate.h>
extern "C" {
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_log.h>
}

mlt_deque ChromixTask::queue = 0;
pthread_t ChromixTask::chromixThread = 0;
pthread_mutex_t ChromixTask::queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ChromixTask::queueCond = PTHREAD_COND_INITIALIZER;
bool ChromixTask::shutdown = false;

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

class ChromixDelegate : public Chromix::Delegate {
public:
    ChromixDelegate(mlt_service service) : service(service) {}

    virtual void logMessage(const std::string& message) {
        //XXX do we need log level too?
        mlt_log(service, MLT_LOG_INFO, message.c_str());
    }
    virtual v8::Handle<v8::Value> getParameterValue(const std::string& name) {
        //XXX can we identify type of property so we wrap it right? need mlt_repository and ID to lookup metadata
        char* value = mlt_properties_get(MLT_SERVICE_PROPERTIES(service), name.c_str());
        if (value)
            return wrapParameterValue(std::string(value));
        return getUndefinedParameterValue();
    }
private:
    mlt_service service;
};

////////////////////////////////

ChromixTask* ChromixTask::getTask(mlt_service service) {
    return (ChromixTask*)mlt_properties_get_data(MLT_SERVICE_PROPERTIES(service), TASK_PROP, NULL);
}

void* ChromixTask::chromixMainLoop(void* arg) {
    AutoChromix chromix(0, NULL);

    pthread_mutex_lock(&queueMutex);
    while (!shutdown) {
        // This manages autorelease pool for Mac
        ChromixContext context;

        ChromixTask *task;
        while ((task = nextQueuedTask()) == 0) {
            pthread_cond_wait(&queueCond, &queueMutex);
            if (shutdown)
                break;
        }
        if (task)
            task->executeTask();
    }

    // Drain the queue
    ChromixTask *task;
    while ((task = nextQueuedTask()))
        task->executeTask();

    pthread_mutex_unlock(&queueMutex);

    pthread_cond_destroy(&queueCond);
    pthread_mutex_destroy(&queueMutex);
    mlt_deque_close(queue);
    //XXX should all this be in AutoChromix? do we even need AutoChromix?
    return NULL;
}

void ChromixTask::shutdownHook(void*) {
    pthread_mutex_lock(&queueMutex);
    shutdown = true;
    pthread_cond_signal(&queueCond);
    pthread_mutex_unlock(&queueMutex);
    pthread_join(chromixThread, NULL);
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
    needsDestruction(false),
    taskResult(0),
    time(0)
{
    pthread_cond_init(&taskCond, NULL);
    mlt_properties_set_data(MLT_SERVICE_PROPERTIES(service), TASK_PROP, this, 0,
                            (mlt_destructor)destroy, NULL);
}

ChromixTask::~ChromixTask() {
    pthread_cond_destroy(&taskCond);
    //XXX assert that mixRender is NULL, it should have been destroyed on chromix thread
}

int ChromixTask::renderToImageForTime(ChromixRawImage& targetImage, double time) {
    this->targetImage = targetImage;
    this->time = time;
    return queueAndWait();
}

int ChromixTask::queueAndWait() {
    MutexLock lock(&queueMutex);
    taskResult = 0;

    // Lazily create queue and thread
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
    mixRender = new Chromix::MixRender(new ChromixDelegate(service));
    if (!mixRender) {
        mlt_log(service, MLT_LOG_ERROR, "failed to create MixRender");
        return 1;
    }
    //XXX load url
    //XXX need to set mixRender properties from mlt_properties_get, iterate over metadata
    //XXX should chromix use a callback to get properties from the app? would need to map to v8 datatype
    bool result = mixRender->loadURL("file:///tmp/test.html");//XXX
    if (!result) {
        mlt_log(service, MLT_LOG_ERROR, "failed to load URL");
        return 1;
    }
    //XXX get and load html page property here, if not specified assume same as service_name".html"

    //XXX for a_track/b_track for transition, "track" for filter, and other tracks
    //XXX add these in yaml as custom props - mapping track to prop name used in html?
    //XXX store this mapping on properties - map char* well defined name (for filter, trans) to WTF::String we can use w/chromix

    //XXX add method dealing with getting the writeable image etc.

    //XXX setup Chromix log callback using mlt_log - should we map chrome levels to mlt? are crhome levels accurate?
    return 0;
}

int ChromixTask::setImageForName(ChromixRawImage& image, const std::string& name) {
    //XXX assert we are on chromix thread
    unsigned char* buffer = mixRender->writeableDataForImageParameter(name, image.width, image.height);
    if (!buffer)
        return 1;
    memcpy(buffer, image.image, image.width * image.height * 4);
    return 0;
}

int ChromixTask::renderToTarget() {
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
