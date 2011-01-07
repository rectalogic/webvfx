// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTCHROMIX_CHROMIX_TASK_H_
#define MLTCHROMIX_CHROMIX_TASK_H_

extern "C" {
    #include <mlt/framework/mlt_service.h>
    #include <mlt/framework/mlt_deque.h>
}
#include <string.h>
#include <pthread.h>
#include <chromix/MixRender.h>


// Image data must be format mlt_image_rgb24a
class ChromixRawImage {
public:
    ChromixRawImage(uint8_t *image=0, int width=0, int height=0) : image(image), width(width), height(height) {}
    void set(uint8_t *image=0, int width=0, int height=0) { this->image = image; this->width = width; this->height = height; }

    uint8_t *image;
    int width;
    int height;
};

////////////////////////////////

class ServiceLock {
public:
    ServiceLock(mlt_service service) : service(service) { mlt_service_lock(service); }
    ~ServiceLock() { mlt_service_unlock(service); }
private:
    mlt_service service;
};

////////////////////////////////

class ChromixTask {
public:

protected:
    ChromixTask(mlt_service service);
    virtual ~ChromixTask();

    // Get task for this service, return NULL if a task needs to be created.
    static ChromixTask* getTask(mlt_service service);

    mlt_service getService() { return service; };

    int renderToImageForTime(ChromixRawImage& targetImage, double time);

    // Can only be called from performTask
    int setImageForName(ChromixRawImage& image, const std::string& name);

    // Subclasses should override to implement Chromix related tasks.
    // Should return a valid MLT error code or 0.
    virtual int performTask() = 0;

private:
    static mlt_deque queue;
    static pthread_mutex_t queueMutex;
    static pthread_cond_t queueCond;
    static pthread_t chromixThread;
    static bool shutdown;

    static void* chromixMainLoop(void*);
    static void shutdownHook(void*);
    static void destroy(ChromixTask *task);
    static ChromixTask* nextQueuedTask() { return (ChromixTask*)mlt_deque_pop_front(queue); };

    mlt_service service;
    Chromix::MixRender *mixRender;
    pthread_cond_t taskCond;
    bool needsDestruction;
    int taskResult;
    double time;
    ChromixRawImage targetImage;

    int queueAndWait();
    int initMixRender();
    int renderToTarget();
    // Execute the task. Must only be called on the chromix thread.
    void executeTask();
};

#endif
