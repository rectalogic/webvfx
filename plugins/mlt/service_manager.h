// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

extern "C" {
#include <framework/mlt_service.h> // for mlt_service_s
#include <framework/mlt_types.h> // for mlt_position, mlt_service, mlt_image
}
#include <cstddef> /* IWYU pragma: keep */ /* IWYU pragma: no_include <ext/type_traits> */ // for byte
#include <vector> // for vector
namespace VfxPipe {
class FrameServer;
}
namespace VfxPipe {
template <typename D>
class VideoFrame;
}
namespace WebVfxPlugin {
class ImageProducer;
}

namespace WebVfxPlugin {

class ServiceManager {
public:
    int render(VfxPipe::VideoFrame<const std::byte*>* vfxSourceImage, VfxPipe::VideoFrame<const std::byte*>* vfxTargetImage, mlt_image outputImage, mlt_position position);

private:
    friend class ServiceLocker;
    ServiceManager(mlt_service service);
    ~ServiceManager();
    bool initialize(mlt_position length);

    mlt_service service;
    mlt_position length;
    VfxPipe::FrameServer* frameServer;
    std::vector<ImageProducer*>* imageProducers;
};

}
