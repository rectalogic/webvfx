// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_VIDEO_TEXTURE_H_
#define WEBVFX_VIDEO_TEXTURE_H_

#include <QtQuick3D/QQuick3DTextureData>

class QVideoSink;

namespace WebVfx {

class VideoTexture : public QQuick3DTextureData {
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ getVideoSink CONSTANT)
    QML_NAMED_ELEMENT(VideoTexture)

public:
    VideoTexture();

    QVideoSink* getVideoSink()
    {
        return videoSink;
    }

private slots:
    void onVideoFrameChanged();

private:
    QVideoSink* videoSink;
};
}
#endif