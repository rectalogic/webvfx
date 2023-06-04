// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <errno.h>
#include <functional>
#include <string>
#include <unistd.h>

namespace VfxPipe {

class VideoFrameFormat {
public:
    enum PixelFormat {
        UNDEFINED = 0,
        RGBA32 = 1,
    };

    VideoFrameFormat(PixelFormat pixelFormat, uint32_t width, uint32_t height, uint32_t stride)
        : pixelFormat(pixelFormat)
        , width(width)
        , height(height)
        , stride(stride)
    {
        if (pixelFormat == PixelFormat::RGBA32)
            dataSize = stride * height * 4;
    }

    VideoFrameFormat(PixelFormat pixelFormat, uint32_t width, uint32_t height)
        : VideoFrameFormat(pixelFormat, width, height, width)
    {
    }

    VideoFrameFormat()
        : VideoFrameFormat(PixelFormat::UNDEFINED, 0, 0, 0)
    {
    }

    bool operator!=(const VideoFrameFormat& other) const
    {
        return !(pixelFormat == other.pixelFormat
            && width == other.width
            && height == other.height
            && stride == other.stride);
    }

    PixelFormat pixelFormat;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t dataSize;
};

class VideoFrame {
public:
    VideoFrame(VideoFrameFormat format, const std::byte* data = nullptr)
        : format(format)
        , data(data)
    {
    }

    VideoFrame(VideoFrameFormat::PixelFormat pixelFormat, uint32_t width, uint32_t height, uint32_t stride, const std::byte* data = nullptr)
        : VideoFrame(VideoFrameFormat(pixelFormat, width, height, stride), data)
    {
    }

    VideoFrame(VideoFrameFormat::PixelFormat pixelFormat, uint32_t width, uint32_t height, const std::byte* data = nullptr)
        : VideoFrame(VideoFrameFormat(pixelFormat, width, height), data)
    {
    }

    VideoFrame()
        : VideoFrame(VideoFrameFormat())
    {
    }

    VideoFrameFormat format;
    const std::byte* data;
};

int spawnProcess(int* pipeRead, int* pipeWrite, const std::string& commandLine, std::function<void(std::string)> errorHandler);

template <typename D, typename IO, typename ERR>
bool dataIO(int fd, D data, size_t size, IO ioFunc, ERR errFunc)
{
    size_t bytesIO = 0;
    while (bytesIO < size) {
        ssize_t n = ioFunc(fd, data + bytesIO, size - bytesIO);
        // EOF
        if (n == 0) {
            errFunc(n);
            return false;
        }
        if (n == -1) {
            errFunc(n, std::string("vfxpipe data IO failed: ") + strerror(errno));
            return false;
        }
        bytesIO = bytesIO + n;
    }
    return true;
}

template <typename ERR>
bool readVideoFrame(int fd, VideoFrame* frame, ERR errFunc)
{
    return dataIO(fd, reinterpret_cast<std::byte*>(&frame->format.pixelFormat), sizeof(frame->format.pixelFormat), read, errFunc)
        && dataIO(fd, reinterpret_cast<std::byte*>(&frame->format.width), sizeof(frame->format.width), read, errFunc)
        && dataIO(fd, reinterpret_cast<std::byte*>(&frame->format.height), sizeof(frame->format.height), read, errFunc)
        && dataIO(fd, reinterpret_cast<std::byte*>(&frame->format.stride), sizeof(frame->format.stride), read, errFunc);
}

template <typename ERR>
bool writeVideoFrame(int fd, VideoFrame* frame, ERR errFunc)
{
    return dataIO(fd, reinterpret_cast<const std::byte*>(&frame->format.pixelFormat), sizeof(frame->format.pixelFormat), write, errFunc)
        && dataIO(fd, reinterpret_cast<const std::byte*>(&frame->format.width), sizeof(frame->format.width), write, errFunc)
        && dataIO(fd, reinterpret_cast<const std::byte*>(&frame->format.height), sizeof(frame->format.height), write, errFunc)
        && dataIO(fd, reinterpret_cast<const std::byte*>(&frame->format.stride), sizeof(frame->format.stride), write, errFunc)
        && (!frame->data || dataIO(fd, frame->data, frame->format.dataSize, write, errFunc));
}

}