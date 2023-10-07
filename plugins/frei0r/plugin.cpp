// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin.h"
#include <cstddef> // for byte
#include <iostream> // for operator<<, basic_ostream, endl, cerr, ostream
#include <stdint.h> // for uint32_t
#include <string> // for string, allocator, operator<<, char_traits
#include <vector> // for vector
#include <vfxpipe.h> // for SourceVideoFrame, FrameServer, VideoFrameFormat, VideoFrameFormat::PixelFormat, VideoFrameFormat::RGBA32, RenderedVideoFrame

class WebVfxPlugin {
public:
    WebVfxPlugin(unsigned int width, unsigned int height)
        : frameServer(nullptr)
        , width(width)
        , height(height)
    {
    }

    ~WebVfxPlugin()
    {
        delete frameServer;
    }

    void setQMLUrl(const char* qmlUrl)
    {
        if (!frameServer) {
            frameServer = new VfxPipe::FrameServer(qmlUrl);
        }
    }

    std::string getQMLUrl()
    {
        if (frameServer)
            return frameServer->getUrl();
        else
            return std::string();
    }

    void updateFrame(
        double time,
        const uint32_t* inframe1,
        const uint32_t* inframe2,
        const uint32_t* inframe3,
        uint32_t* outframe)
    {
        if (!frameServer)
            return;

        auto errorHandler = [](std::string msg) {
            std::cerr << __FUNCTION__ << ": " << msg << std::endl;
        };

        std::vector<VfxPipe::SourceVideoFrame> sourceFrames;
        if (inframe1)
            sourceFrames.emplace_back(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height, reinterpret_cast<const std::byte*>(inframe1));
        if (inframe2)
            sourceFrames.emplace_back(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height, reinterpret_cast<const std::byte*>(inframe2));
        if (inframe3)
            sourceFrames.emplace_back(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height, reinterpret_cast<const std::byte*>(inframe3));
        VfxPipe::RenderedVideoFrame outputFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, width, height, reinterpret_cast<std::byte*>(outframe));
        if (!frameServer->renderFrame(time, sourceFrames, outputFrame, errorHandler)) {
            delete frameServer;
            frameServer = nullptr;
        }
    }

private:
    VfxPipe::FrameServer* frameServer;
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