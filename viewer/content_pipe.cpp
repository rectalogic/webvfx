// Copyright (c) 2023 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content_pipe.h"
#include <QImage>
#include <QtDebug>
#include <QtGlobal>
#include <stdio.h>
#include <vfxpipe.h>

ContentPipe::ContentPipe(const QUrl& url, QObject* parent)
    : QObject(parent)
    , url(url)
    , pid(0)
    , pipeWrite(-1)
    , pipeRead(-1)
{
}

ContentPipe::~ContentPipe()
{
    if (pipeRead != -1)
        close(pipeRead);
    if (pipeWrite != -1)
        close(pipeWrite);
}

bool ContentPipe::renderContent(
    double time,
    const QList<QImage> sourceImages,
    QImage& outputImage)
{
    Q_ASSERT(outputImage.format() == QImage::Format_RGBA8888);

    if (!pid) {
        auto spawnErrorHandler = [](std::string msg) {
            qCritical() << __FUNCTION__ << ": " << msg;
        };
        pid = VfxPipe::spawnProcess(&pipeRead, &pipeWrite, url.toString().toStdString(), spawnErrorHandler);
    }
    if (pid == -1) {
        qCritical() << __FUNCTION__ << ": vfxpipe failed to spawn process";
        return false;
    }

    auto ioErrorHandler = [this](int n, std::string msg = "") {
        if (!msg.empty())
            qCritical() << __FUNCTION__ << ": " << msg;
        close(pipeRead);
        pipeRead = -1;
        close(pipeWrite);
        pipeWrite = -1;
    };

    if (!VfxPipe::dataIO(pipeWrite, reinterpret_cast<std::byte*>(&time), sizeof(time), write, ioErrorHandler)) {
        return false;
    }

    // Output format
    VfxPipe::VideoFrame vfxOutputFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, outputImage.width(), outputImage.height());
    if (!VfxPipe::writeVideoFrame(pipeWrite, &vfxOutputFrame, ioErrorHandler)) {
        return false;
    }

    uint32_t frameCount = sourceImages.size();
    if (!VfxPipe::dataIO(pipeWrite, reinterpret_cast<const std::byte*>(&frameCount), sizeof(frameCount), write, ioErrorHandler)) {
        return false;
    }
    for (const auto& sourceImage : sourceImages) {
        Q_ASSERT(sourceImage.format() == QImage::Format_RGBA8888);
        VfxPipe::VideoFrame vfxFrame(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, sourceImage.width(), sourceImage.height(), reinterpret_cast<const std::byte*>(sourceImage.constBits()));
        if (!VfxPipe::writeVideoFrame(pipeWrite, &vfxFrame, ioErrorHandler)) {
            return false;
        }
    }

    if (!VfxPipe::dataIO(pipeRead, reinterpret_cast<std::byte*>(outputImage.bits()), vfxOutputFrame.format.dataSize(), read, ioErrorHandler)) {
        return false;
    }
}