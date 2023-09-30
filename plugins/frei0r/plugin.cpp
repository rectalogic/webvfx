// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin.h"
#include <cstring>
#include <dlfcn.h>
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vfxpipe.h>
#include <wordexp.h>

class WebVfxPlugin {
public:
    WebVfxPlugin(unsigned int width, unsigned int height)
        : width(width)
        , height(height)
        , pid(0)
        , pipeWrite(-1)
        , pipeRead(-1)
    {
    }

    ~WebVfxPlugin()
    {
        if (pipeRead != -1)
            close(pipeRead);
        if (pipeWrite != -1)
            close(pipeWrite);
    }

    void setQMLUrl(const char* qmlUrl)
    {
        if (this->qmlUrl.empty()) {
            this->qmlUrl = qmlUrl;
        }
    }

    std::string& getQMLUrl()
    {
        return this->qmlUrl;
    }

    void updateFrame(
        double time,
        const uint32_t* inframe1,
        const uint32_t* inframe2,
        const uint32_t* inframe3,
        uint32_t* outframe)
    {
        if (!pid) {
            auto spawnErrorHandler = [](std::string msg) {
                std::cerr << __FUNCTION__ << ": " << msg << std::endl;
            };
            pid = VfxPipe::spawnProcess(&pipeRead, &pipeWrite, qmlUrl, spawnErrorHandler);
        }
        if (pid == -1) {
            std::cerr << __FUNCTION__ << ": vfxpipe failed to spawn process" << std::endl;
            return;
        }

        auto ioErrorHandler = [this](int n, std::string msg = "") {
            if (!msg.empty())
                std::cerr << __FUNCTION__ << ": " << msg << std::endl;
            close(pipeRead);
            pipeRead = -1;
            close(pipeWrite);
            pipeWrite = -1;
        };

        if (!VfxPipe::dataIO(pipeWrite, reinterpret_cast<std::byte*>(&time), sizeof(time), write, ioErrorHandler)) {
            return;
        }

        // Output format
        VfxPipe::VideoFrame vfxOutputFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height);
        if (!VfxPipe::writeVideoFrame(pipeWrite, &vfxOutputFrame, ioErrorHandler)) {
            return;
        }

        uint32_t frameCount = 0;
        if (inframe1)
            frameCount++;
        if (inframe2)
            frameCount++;
        if (inframe3)
            frameCount++;
        if (!VfxPipe::dataIO(pipeWrite, reinterpret_cast<const std::byte*>(&frameCount), sizeof(frameCount), write, ioErrorHandler)) {
            return;
        }

        if (inframe1) {
            VfxPipe::VideoFrame vfxFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height, reinterpret_cast<const std::byte*>(inframe1));
            if (!VfxPipe::writeVideoFrame(pipeWrite, &vfxFrame, ioErrorHandler)) {
                return;
            }
        }
        if (inframe2) {
            VfxPipe::VideoFrame vfxFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height, reinterpret_cast<const std::byte*>(inframe2));
            if (!VfxPipe::writeVideoFrame(pipeWrite, &vfxFrame, ioErrorHandler)) {
                return;
            }
        }
        if (inframe3) {
            VfxPipe::VideoFrame vfxFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height, reinterpret_cast<const std::byte*>(inframe3));
            if (!VfxPipe::writeVideoFrame(pipeWrite, &vfxFrame, ioErrorHandler)) {
                return;
            }
        }

        if (!VfxPipe::dataIO(pipeRead, reinterpret_cast<std::byte*>(outframe), vfxOutputFrame.format.dataSize(), read, ioErrorHandler)) {
            return;
        }
    }

private:
    int pid;
    int pipeWrite;
    int pipeRead;
    std::string qmlUrl;
    unsigned int width;
    unsigned int height;
};

int f0r_init()
{
    return 1;
}

void f0r_deinit()
{
}

void get_common_plugin_info(f0r_plugin_info_t* info)
{
    info->author = "Andrew Wason";
    info->color_model = F0R_COLOR_MODEL_RGBA8888;
    info->frei0r_version = FREI0R_MAJOR_VERSION;
    info->major_version = 0;
    info->minor_version = 9;
    info->num_params = 1;
}

void f0r_get_param_info(f0r_param_info_t* info, int param_index)
{
    if (param_index == 0) {
        info->name = "QML URL";
        info->type = F0R_PARAM_STRING;
        info->explanation = "QML url to load and pipe frames to/from";
    }
}

f0r_instance_t f0r_construct(unsigned int width, unsigned int height)
{
    return static_cast<f0r_instance_t>(new WebVfxPlugin(width, height));
}

void f0r_destruct(f0r_instance_t instance)
{
    WebVfxPlugin* vfxpipe = static_cast<WebVfxPlugin*>(instance);
    delete vfxpipe;
}

void f0r_set_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
    if (param_index != 0)
        return;
    WebVfxPlugin* vfxpipe = static_cast<WebVfxPlugin*>(instance);
    vfxpipe->setQMLUrl(*(static_cast<f0r_param_string*>(param)));
}

void f0r_get_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
    if (param_index != 0)
        return;
    WebVfxPlugin* vfxpipe = static_cast<WebVfxPlugin*>(instance);
    *static_cast<f0r_param_string*>(param) = const_cast<f0r_param_string>(vfxpipe->getQMLUrl().c_str());
}

void f0r_update2(
    f0r_instance_t instance,
    double time,
    const uint32_t* inframe1,
    const uint32_t* inframe2,
    const uint32_t* inframe3,
    uint32_t* outframe)
{
    WebVfxPlugin* vfxpipe = static_cast<WebVfxPlugin*>(instance);
    vfxpipe->updateFrame(time, inframe1, inframe2, inframe3, outframe);
}

void f0r_update(
    f0r_instance_t instance,
    double time,
    const uint32_t* inframe,
    uint32_t* outframe)
{
    f0r_update2(instance, time, inframe, 0, 0, outframe);
}