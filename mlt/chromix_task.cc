// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix_task.h"
#include "chromix_service.h"
#include "chromix_context.h"
#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
#include <chromix/Delegate.h>
extern "C" {
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_producer.h>
    #include <mlt/framework/mlt_frame.h>
    #include <mlt/framework/mlt_log.h>
}

mlt_deque ChromixTask::queue = 0;
pthread_t ChromixTask::chromixThread = 0;
pthread_mutex_t ChromixTask::queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ChromixTask::queueCond = PTHREAD_COND_INITIALIZER;
bool ChromixTask::shutdown = false;

#define TASK_PROP "ChromixTask"
#define CHROMIX_PRODUCERS_PROP "chromix.producers"

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

class ProducerImage {
public:
    ProducerImage(const std::string& name, mlt_producer producer) : name(name), producer(producer) {}
    ~ProducerImage() {
        mlt_producer_close(producer);
    }
    int produceImage(mlt_position position, int targetWidth, int targetHeight) {
        if (position > mlt_producer_get_out(producer))
            return 0;
        mlt_producer_seek(producer, position);
        mlt_frame frame = NULL;
        int error = mlt_service_get_frame(MLT_PRODUCER_SERVICE(producer), &frame, 0);
        if (error)
            return error;
        mlt_frame_set_position(frame, position);

        mlt_image_format format = mlt_image_rgb24a;
        uint8_t *image = NULL;
        int width = targetWidth;
        int height = targetHeight;
        error = mlt_frame_get_image(frame, &image, &format, &width, &height, 0);
        if (!error)
            rawImage.set(image, width, height);
        mlt_frame_close(frame);
        return error;
    }
    const std::string& getName() { return name; }
    const ChromixRawImage& getImage() { return rawImage; }
    void reset() { rawImage.set(); }

private:
    std::string name;
    mlt_producer producer;
    ChromixRawImage rawImage;
};

////////////////////////////////

class ChromixDelegate : public Chromix::Delegate {
public:
    ChromixDelegate(mlt_service service) : service(service) {}

    virtual void logMessage(const std::string& message) {
        //XXX do we need log level too - map Chromium levels to mlt?
        mlt_log(service, MLT_LOG_INFO, "%s\n", message.c_str());
    }
    virtual v8::Handle<v8::Value> getParameterValue(const std::string& name) {
        //XXX parse metadata params in initialize and map to datatype, then set proper type wrapper here
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

ChromixTask::ChromixTask(mlt_service service, const std::string& serviceName) :
    serviceName(serviceName),
    service(service),
    metadata(NULL),
    mixRender(0),
    needsDestruction(false),
    taskResult(0),
    time(0),
    producerImages(NULL)
{
    pthread_cond_init(&taskCond, NULL);
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);
    mlt_properties_set_data(properties, TASK_PROP, this, 0, (mlt_destructor)destroy, NULL);
    mlt_properties_set(properties, "factory", mlt_environment("MLT_PRODUCER"));
}

ChromixTask::~ChromixTask() {
    pthread_cond_destroy(&taskCond);
    //XXX assert that mixRender is NULL, it should have been destroyed on chromix thread
    if (metadata)
        mlt_properties_close(metadata);
    if (producerImages) {
        for (std::vector<ProducerImage*>::iterator pi = producerImages->begin();
             pi != producerImages->end(); pi++) {
            delete *pi;
        }
        delete producerImages;
    }
}

int ChromixTask::initialize() {
    metadata = chromix_load_metadata(serviceName);
    if (!metadata) {
        mlt_log(service, MLT_LOG_FATAL, "failed to load metadata\n");
        return 1;
    }
    return createProducerImages();
}

int ChromixTask::createProducerImages() {
    mlt_properties producers = (mlt_properties)mlt_properties_get_data(metadata, CHROMIX_PRODUCERS_PROP, NULL);
    if (!producers)
        return 0;

    int count = mlt_properties_count(producers);
    // Warn if no values, but not an error
    if (count == 0) {
        mlt_log(service, MLT_LOG_WARNING, "no list defined for " CHROMIX_PRODUCERS_PROP " in metadata\n");
        return 0;
    }

    producerImages = new std::vector<ProducerImage*>(count, NULL);
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);
    char* factory = mlt_properties_get(properties, "factory");
    int i;
    for (i = 0; i < count; i++) {
        std::string producerName(mlt_properties_get_value(producers, i));
        // Property prefix "producer.<name>."
        std::string producerPrefix("producer.");
        producerPrefix.append(producerName).append(".");

        // Find producer.<name>.resource property
        std::string resourceName(producerPrefix);
        resourceName.append("resource");
        char* resource = mlt_properties_get(properties, resourceName.c_str());
        if (resource) {
            mlt_producer producer = mlt_factory_producer(mlt_service_profile(service), factory, resource);
            if (!producer) {
                mlt_log(service, MLT_LOG_ERROR, "failed to create producer for %s\n", resourceName.c_str());
                return 1;
            }
            // Copy producer.<name>.* properties onto producer
            mlt_properties_pass(MLT_PRODUCER_PROPERTIES(producer), properties, producerPrefix.c_str());
            (*producerImages)[i] = new ProducerImage(producerName, producer);
        }
        else
            mlt_log(service, MLT_LOG_WARNING, "no producer resource property specified for %s\n", resourceName.c_str());
    }
    return 0;
}

int ChromixTask::renderToImageForPosition(ChromixRawImage& targetImage, mlt_position position) {
    int error = 0;

    // Perform one-time initialization
    if (!metadata) {
        error = initialize();
        if (error)
            return error;
    }

    // Compute time
    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);
    mlt_position in = mlt_properties_get_position(properties, "in");
    mlt_position length = mlt_properties_get_position(properties, "out") - in + 1;
    this->time = (double)(position - in) / (double)length;

    if (producerImages) {
        error = writeProducerImages(position, targetImage.getWidth(), targetImage.getHeight());
        if (error)
            return error;
    }

    this->targetImage = targetImage;
    error = queueAndWait();
    return error;
}

int ChromixTask::writeProducerImages(mlt_position position, int width, int height) {
    int error = 0;
    for (std::vector<ProducerImage*>::iterator pi = producerImages->begin();
         pi != producerImages->end(); pi++) {
        ProducerImage* producerImage = *pi;
        if (producerImage) {
            error = producerImage->produceImage(position, width, height);
            if (error) {
                mlt_log(service, MLT_LOG_ERROR, "failed to produce image for name %s\n", producerImage->getName().c_str());
                return error;
            }
        }
    }
    return error;
}

int ChromixTask::queueAndWait() {
    MutexLock lock(&queueMutex);
    taskResult = 0;

    // Lazily create queue and thread
    if (chromixThread == 0) {
        queue = mlt_deque_init();
        if (pthread_create(&chromixThread, NULL, chromixMainLoop, 0) < 0) {
            mlt_log(NULL, MLT_LOG_FATAL, "failed to spawn Chromix thread\n");
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
        mlt_log(service, MLT_LOG_ERROR, "failed to create MixRender\n");
        return 1;
    }

    // Load HTML. Look up relative path in service metadata.
    char *htmlPath = mlt_properties_get(metadata, HTML_METADATA_PROP);
    if (!htmlPath) {
        mlt_log(service, MLT_LOG_ERROR, "property '" HTML_METADATA_PROP "' not found in metadata\n");
        return 1;
    }
    std::string url(std::string("file://") + chromix_get_metadata_dir() + htmlPath);
    bool result = mixRender->loadURL(url);
    if (!result) {
        mlt_log(service, MLT_LOG_ERROR, "failed to load URL %s\n", url.c_str());
        return 1;
    }

    return 0;
}

int ChromixTask::setImageForName(const ChromixRawImage& image, const std::string& name) {
    //XXX assert we are on chromix thread
    if (!image.valid())
        return 0;
    unsigned char* buffer = mixRender->writeableDataForImageParameter(name, image.getWidth(), image.getHeight());
    if (!buffer)
        return 1;
    image.copy(buffer);
    return 0;
}

int ChromixTask::renderToTarget() {
    // Resize MixRender and render into target image
    mixRender->resize(targetImage.getWidth(), targetImage.getHeight());
    //XXX move this into MixRender - pass raw buffer for it to render into
    const SkBitmap* skiaBitmap = mixRender->render(time);
    if (!skiaBitmap)
        return 1;
    SkAutoLockPixels bitmapLock(*skiaBitmap);
    // Do a block copy if no padding, otherwise copy a row at a time
    unsigned int byteCount = targetImage.getWidth() * targetImage.getHeight() * 4;
    if (skiaBitmap->getSize() == byteCount)
        memcpy(targetImage.getImage(), skiaBitmap->getPixels(), byteCount);
    else {
        int bytesPerRow = targetImage.getWidth() * 4;
        const unsigned char* srcP = reinterpret_cast<const unsigned char*>(skiaBitmap->getPixels());
        unsigned char* dstP = targetImage.getImage();
        for (int y = 0; y < targetImage.getHeight(); y++) {
            memcpy(dstP, srcP, bytesPerRow);
            srcP += skiaBitmap->rowBytes();
            dstP += bytesPerRow;
        }
    }
    targetImage.set();
    return 0;
}

int ChromixTask::storeProducerImages() {
    for (std::vector<ProducerImage*>::iterator pi = producerImages->begin();
         pi != producerImages->end(); pi++) {
        ProducerImage* producerImage = *pi;
        if (producerImage) {
            int error = setImageForName(producerImage->getImage(), producerImage->getName());
            if (error)
                return error;
            producerImage->reset();
        }
    }
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
            if (producerImages)
                taskResult = storeProducerImages();
            if (taskResult == 0) {
                taskResult = performTask();
                if (taskResult == 0)
                    taskResult = renderToTarget();
            }
        }
        pthread_cond_signal(&taskCond);
    }
}
