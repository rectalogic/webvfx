// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <iterator>

extern "C" {
    #include <framework/mlt_log.h>
    #include <framework/mlt_factory.h>
    #include <framework/mlt_frame.h>
    #include <framework/mlt_image.h>
    #include <framework/mlt_producer.h>
}
#include "service_manager.h"

extern char **environ;

namespace VFXPipe
{

class ImageProducer
{
public:
    ImageProducer( mlt_producer producer)
        : producerFrame(0)
        , producer(producer) {}

    ~ImageProducer() {
        if (producerFrame)
            mlt_frame_close(producerFrame);
        mlt_producer_close(producer);
    }

    bool isPositionValid(mlt_position position) {
        return position < mlt_producer_get_playtime(producer);
    }

    mlt_image_s produceImage(mlt_position position, int width, int height) {
        // Close previous frame and request a new one.
        // We don't close the current frame because the image data we return
        // needs to remain valid until we are rendered.
        if (producerFrame) {
            mlt_frame_close(producerFrame);
            producerFrame = 0;
        }
        mlt_producer_seek(producer, position);
        mlt_service_get_frame(MLT_PRODUCER_SERVICE(producer), &producerFrame, 0);

        mlt_image_format format = mlt_image_rgb;
        uint8_t *image = NULL;
        int error = mlt_frame_get_image(producerFrame, &image, &format,
                                        &width, &height, 0);
        if (error)
            return mlt_image_s();
        mlt_image_s producedImage;
        mlt_image_set_values(&producedImage, image, format, width, height);
        return producedImage;
    }

private:
    mlt_frame producerFrame;
    mlt_producer producer;
};

////////////////////////

template <typename T>
static bool dataIO(int fd, void *data, size_t size, T ioFunc) {
    size_t bytesIO = 0;
    while (bytesIO < size) {
        ssize_t n = ioFunc(fd, static_cast<std::byte *>(data) + bytesIO, size - bytesIO);
        if (n < 0) {
            return false;
        }
        bytesIO = bytesIO + n;
    }
    return true;
}

////////////////////////

ServiceManager::ServiceManager(mlt_service service)
    : service(service)
    , pipeRead(-1)
    , pipeWrite(-1)
    , imageProducers(0)
{
    mlt_properties_set(MLT_SERVICE_PROPERTIES(service), "factory", mlt_environment("MLT_PRODUCER"));
}

ServiceManager::~ServiceManager()
{
    if (pipeRead != -1)
        close(pipeRead);
    if (pipeWrite != -1)
        close(pipeWrite);
    if (imageProducers) {
        for (std::vector<ImageProducer*>::iterator it = imageProducers->begin();
             it != imageProducers->end(); it++) {
            delete *it;
        }
        delete imageProducers;
    }
}

bool ServiceManager::initialize(int width, int height, mlt_position length)
{
    // Return if already initialized
    if (pid == -1 || pid > 0)
        return true;

    mlt_properties properties = MLT_SERVICE_PROPERTIES(service);

    // Create and initialize Effects
    char* command = mlt_properties_get(properties, "resource");
    if (!command) {
        pid = -1;
        mlt_log_error(service, "No 'resource' property found\n");
        return false;
    }

    int fdsToChild[2];
    int fdsFromChild[2];

    if (pipe(fdsToChild) == -1
        || pipe(fdsFromChild) == -1) {
        pid = -1;
        mlt_log_error(service, "%s: vfxpipe pipe failed: %s\n", __FUNCTION__, strerror(errno));
        return false;
    }

    // Ignore child exit so we don't have to waitpid, and to avoid zombie processes
    signal(SIGCHLD, SIG_IGN);

    pid = fork();
    if (pid == -1) {
        mlt_log_error(service, "%s: vfxpipe fork failed: %s\n", __FUNCTION__, strerror(errno));
        return false;
    }
    // In the child
    if (pid == 0) {
        if (dup2(fdsToChild[0], STDIN_FILENO) == -1
            || dup2(fdsFromChild[1], STDOUT_FILENO) == -1) {
            mlt_log_error(service, "%s: vfxpipe dup2 failed: %s\n", __FUNCTION__, strerror(errno));
            exit(1);
        }

		close(fdsFromChild[0]);
		close(fdsFromChild[1]);
		close(fdsToChild[0]);
		close(fdsToChild[1]);

        auto envWidth = std::string("VFXPIPE_WIDTH=") + std::to_string(width);
        auto envHeight = std::string("VFXPIPE_HEIGHT=") + std::to_string(height);
        auto envFrameCount = std::string("VFXPIPE_FRAMECOUNT=") + std::to_string(length);
        const char * const envExtra[] = {
            envWidth.c_str(),
            envHeight.c_str(),
            envFrameCount.c_str(),
            NULL,
        };
        char **p;
        int environSize;
        for (p = environ, environSize = 0; *p != NULL; p++, environSize++);
        char const * envp[environSize + std::size(envExtra)];
        for (auto i = 0; i < environSize; i++) {
            envp[i] = environ[i];
        }
        for (size_t i = environSize, j = 0; j < std::size(envExtra); i++, j++) {
            envp[i] = envExtra[j];
        }
        auto execCommand = std::string("exec ") + command;
        const char * const argv[] = {
            "/bin/sh",
            "-c",
            execCommand.c_str(),
            NULL,
        };
        if (execve(argv[0], const_cast<char * const *>(argv), const_cast<char * const *>(envp)) < 0) {
            mlt_log_error(service, "%s: vfxpipe exec failed: %s\n", __FUNCTION__, strerror(errno));
            exit(1);
        }
    }

    // In the parent

    pipeWrite = fdsToChild[1];
    pipeRead = fdsFromChild[0];

    close(fdsFromChild[1]);
	close(fdsToChild[0]);

    //XXX fork/exec in here
    if (1)
        return true;

    //XXX fork/exec - save pipes

    //XXX iterate producer.N.resource properties and create ImageProducer, copy stuff over
#if 0
    // Iterate over image map - save source and target image names,
    // and create an ImageProducer for each extra image.
    char* factory = mlt_properties_get(properties, "factory");
    WebVfx::Effects::ImageTypeMapIterator it(effects->getImageTypeMap());
    while (it.hasNext()) {
        it.next();

        const QString& imageName = it.key();

        switch (it.value()) {

        case WebVfx::Effects::SourceImageType:
            sourceImageName = imageName;
            break;

        case WebVfx::Effects::TargetImageType:
            targetImageName = imageName;
            break;

        case WebVfx::Effects::ExtraImageType:
        {
            if (!imageProducers)
                imageProducers = new std::vector<ImageProducer*>(3);

            // Property prefix "producer.<name>."
            QString producerPrefix("producer.");
            producerPrefix.append(imageName).append(".");

            // Find producer.<name>.resource property
            QString resourceName(producerPrefix);
            resourceName.append("resource");
            char* resource = mlt_properties_get(properties, resourceName.toLatin1().constData());
            if (resource) {
                mlt_producer producer = mlt_factory_producer(mlt_service_profile(service), factory, resource);
                if (!producer) {
                    mlt_log(service, MLT_LOG_ERROR, "VFXPipe failed to create extra image producer for %s\n", resourceName.toLatin1().constData());
                    return false;
                }
                // Copy producer.<name>.* properties onto producer
                mlt_properties_pass(MLT_PRODUCER_PROPERTIES(producer), properties, producerPrefix.toLatin1().constData());
                // Append ImageProducer to vector
                imageProducers->insert(imageProducers->end(), new ImageProducer(imageName, producer));
            }
            else
                mlt_log(service, MLT_LOG_WARNING, "VFXPipe no producer resource property specified for extra image %s\n", resourceName.toLatin1().constData());
            break;
        }

        default:
            mlt_log(service, MLT_LOG_ERROR, "Invalid VFXPipe image type %d\n", it.value());
            break;
        }
    }
#endif
    return true;
}

int ServiceManager::render(mlt_image sourceImage, mlt_image targetImage, mlt_image outputImage)
{
    if (pipeRead == -1 || pipeWrite == -1)
        return 1;
    if (sourceImage != nullptr) {
        if (!dataIO(pipeWrite, sourceImage->data, mlt_image_calculate_size(sourceImage), write)) {
            mlt_log_error(service, "%s: Failed to write source image: %s\n", __FUNCTION__, strerror(errno));
            return 1;
        }
    }
    if (targetImage != nullptr) {
        if (!dataIO(pipeWrite, targetImage->data, mlt_image_calculate_size(targetImage), write)) {
            mlt_log_error(service, "%s: Failed to write target image: %s\n", __FUNCTION__, strerror(errno));
            return 1;
        }
    }
    //XXX write producer extra images
    if (!dataIO(pipeRead, outputImage->data, mlt_image_calculate_size(outputImage), read)) {
        mlt_log_error(service, "%s: Failed to read output image: %s\n", __FUNCTION__, strerror(errno));
        return 1;
    }
    return 0;
#if 0
    double time = length > 0 ? position / (double)length : 0;

    //XXX write source/target to pipe
    return !effects->render(time, outputImage);

    // Produce any extra images
    if (imageProducers) {
        for (std::vector<ImageProducer*>::iterator it = imageProducers->begin();
             it != imageProducers->end(); it++) {
            ImageProducer* imageProducer = *it;
            if (imageProducer && imageProducer->isPositionValid(position)) {
                mlt_image_s extraImage =
                    imageProducer->produceImage(position,
                                                outputImage->width(),
                                                outputImage->height());
                if (!extraImage.data) {
                    mlt_log(service, MLT_LOG_ERROR, "WebVfx failed to produce image for name %s\n", imageProducer->getName().toLatin1().constData());
                    return 1;
                }
                effects->setImage(imageProducer->getName(), &extraImage); //XXX write to pipe
            }
        }
    }
#endif
}

}
