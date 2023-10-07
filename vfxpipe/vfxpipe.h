// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef> /* IWYU pragma: keep */ /* IWYU pragma: no_include <ext/type_traits> */ // for byte, size_t
#include <errno.h> // for errno
#include <functional> // for function
#include <stdint.h> // for uint32_t
#include <string.h> // for strerror
#include <string> // for string, allocator, operator+
#include <unistd.h> // for write, read, size_t, ssize_t
#include <vector> // for vector

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
    }

    VideoFrameFormat(PixelFormat pixelFormat, uint32_t width, uint32_t height)
        : VideoFrameFormat(pixelFormat, width, height, width)
    {
    }

    VideoFrameFormat()
        : VideoFrameFormat(PixelFormat::UNDEFINED, 0, 0, 0)
    {
    }

    size_t dataSize() const
    {
        if (pixelFormat == PixelFormat::RGBA32)
            return stride * height * 4;
        else
            return 0;
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
};

template <typename D>
class VideoFrame {
public:
    VideoFrame(VideoFrameFormat format, D data = nullptr)
        : format(format)
        , data(data)
    {
    }

    VideoFrame(VideoFrameFormat::PixelFormat pixelFormat, uint32_t width, uint32_t height, uint32_t stride, D data = nullptr)
        : VideoFrame(VideoFrameFormat(pixelFormat, width, height, stride), data)
    {
    }

    VideoFrame(VideoFrameFormat::PixelFormat pixelFormat, uint32_t width, uint32_t height, D data = nullptr)
        : VideoFrame(VideoFrameFormat(pixelFormat, width, height), data)
    {
    }

    VideoFrame()
        : VideoFrame(VideoFrameFormat())
    {
    }

    VideoFrameFormat format;
    D data;
};

using SourceVideoFrame = VideoFrame<const std::byte*>;
using RenderedVideoFrame = VideoFrame<std::byte*>;
using ErrorHandler = std::function<void(std::string)>;

class FrameServer {
public:
    FrameServer(const std::string& url);
    ~FrameServer();
    bool renderFrame(double time, const std::vector<SourceVideoFrame>& sourceFrames, RenderedVideoFrame& outputFrame, ErrorHandler errorHandler);
    std::string& getUrl() { return url; }

private:
    std::string url;
    int pid;
    int pipeWrite;
    int pipeRead;
};

template <typename D, typename IO>
bool dataIO(int fd, D data, size_t size, IO ioFunc, ErrorHandler errFunc)
{
    size_t bytesIO = 0;
    while (bytesIO < size) {
        ssize_t n = ioFunc(fd, data + bytesIO, size - bytesIO);
        // EOF
        if (n == 0) {
            errFunc("");
            return false;
        }
        if (n == -1) {
            errFunc(std::string("vfxpipe data IO failed: ") + strerror(errno));
            return false;
        }
        bytesIO = bytesIO + n;
    }
    return true;
}

inline bool readVideoFrameFormat(int fd, VideoFrameFormat& format, ErrorHandler errFunc)
{
    return dataIO(fd, reinterpret_cast<std::byte*>(&format.pixelFormat), sizeof(format.pixelFormat), read, errFunc)
        && dataIO(fd, reinterpret_cast<std::byte*>(&format.width), sizeof(format.width), read, errFunc)
        && dataIO(fd, reinterpret_cast<std::byte*>(&format.height), sizeof(format.height), read, errFunc)
        && dataIO(fd, reinterpret_cast<std::byte*>(&format.stride), sizeof(format.stride), read, errFunc);
}

inline bool writeVideoFrameFormat(int fd, const VideoFrameFormat& format, ErrorHandler errFunc)
{
    return dataIO(fd, reinterpret_cast<const std::byte*>(&format.pixelFormat), sizeof(format.pixelFormat), write, errFunc)
        && dataIO(fd, reinterpret_cast<const std::byte*>(&format.width), sizeof(format.width), write, errFunc)
        && dataIO(fd, reinterpret_cast<const std::byte*>(&format.height), sizeof(format.height), write, errFunc)
        && dataIO(fd, reinterpret_cast<const std::byte*>(&format.stride), sizeof(format.stride), write, errFunc);
}

inline bool writeVideoFrame(int fd, const SourceVideoFrame& frame, ErrorHandler errFunc)
{
    return writeVideoFrameFormat(fd, frame.format, errFunc)
        && (!frame.data || dataIO(fd, frame.data, frame.format.dataSize(), write, errFunc));
}

}