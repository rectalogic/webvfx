// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTCHROMIX_CHROMIX_TASK_H_
#define MLTCHROMIX_CHROMIX_TASK_H_

extern "C" {
    #include <mlt/framework/mlt_service.h>
    #include <mlt/framework/mlt_deque.h>
}
#include <chromix/mix_render.h>

#include <string>
#include <vector>
#include <pthread.h>



#define HTML_METADATA_PROP "chromix.html"
#define A_IMAGE_METADATA_PROP "chromix.a_image"
#define B_IMAGE_METADATA_PROP "chromix.b_image"

// Image data must be format mlt_image_rgb24a
class ChromixRawImage {
public:
    ChromixRawImage(uint8_t *image=0, int width=0, int height=0) : image(image), width(width), height(height) {}
    void set(uint8_t *image=0, int width=0, int height=0) { this->image = image; this->width = width; this->height = height; }
    bool valid() const { return image; }
    void copy(uint8_t *buffer) const { memcpy(buffer, image, width * height * 4); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    uint8_t* getImage() { return image; }

private:
    uint8_t* image;
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

class ProducerImage;

class ChromixTask {
protected:
    ChromixTask(mlt_service service, const std::string& serviceName);
    virtual ~ChromixTask();

    // Subclasses should override for one time initialization, and must call superclass implementation.
    virtual int initialize();

    // Get task for this service
    static ChromixTask* getTask(mlt_service service);

    mlt_service getService() { return service; };
    mlt_properties getMetadata() { return metadata; };

    int renderToImageForPosition(ChromixRawImage& targetImage, mlt_position position);

    // Can only be called from performTask
    int setImageForName(const ChromixRawImage& image, const std::string& name);

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
    std::string serviceName;
    mlt_properties metadata;
    Chromix::MixRender *mixRender;
    pthread_cond_t taskCond;
    bool needsDestruction;
    int taskResult;
    double time;
    ChromixRawImage targetImage;
    std::vector<ProducerImage*>* producerImages;

    int createProducerImages();
    int writeProducerImages(mlt_position position, int targetWidth, int targetHeight);
    int queueAndWait();
    int initMixRender();
    int renderToTarget();
    int storeProducerImages();
    // Execute the task. Must only be called on the chromix thread.
    void executeTask();
};

#endif
