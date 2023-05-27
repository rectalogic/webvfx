// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_texture.h"
#include "webvfx.h"
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>
#include <QtDebug>

namespace WebVfx {

VideoTextureData::VideoTextureData()
    : videoSink(new QVideoSink(this))
{
    connect(videoSink, SIGNAL(videoFrameChanged(const QVideoFrame&)), SLOT(onVideoFrameChanged()));
}

void VideoTextureData::onVideoFrameChanged()
{
    QVideoFrame frame = videoSink->videoFrame();
    if (frame.pixelFormat() != QVideoFrameFormat::Format_RGBA8888) {
        qDebug() << "Unable to handle QVideoFrame pixel format";
        return;
    }
    frame.map(QVideoFrame::ReadOnly);
    setSize(frame.size());
    setHasTransparency(true);
    setFormat(QQuick3DTextureData::RGBA8);
    setTextureData(QByteArray::fromRawData((const char*)frame.bits(0), frame.mappedBytes(0)));
    frame.unmap();
    update();
}
}