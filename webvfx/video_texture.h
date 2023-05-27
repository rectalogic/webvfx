// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QtQuick3D/QQuick3DTextureData>

class QVideoSink;

namespace WebVfx {

class VideoTextureData : public QQuick3DTextureData {
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ getVideoSink CONSTANT)
    QML_NAMED_ELEMENT(VideoTextureData)

public:
    VideoTextureData();

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
