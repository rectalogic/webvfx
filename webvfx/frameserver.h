// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QImage>
#include <QList>
#include <QObject>
#include <QSize>
#include <QVideoFrame>
#include <vfxpipe.h>

class QVideoSink;

namespace WebVfx {
class QmlContent;
}

struct FrameSink {
    FrameSink(QVideoSink* sink)
        : sink(sink) {};
    VfxPipe::VideoFrameFormat format;
    QVideoFrame frames[2];
    QVideoSink* sink;
};

class FrameServer : public QObject {
    Q_OBJECT

public:
    FrameServer(QUrl& qmlUrl, double duration = 0, QObject* parent = nullptr);
    ~FrameServer();

    bool event(QEvent* event) override;

private slots:
    void onContentLoadFinished(bool);

private:
    void readFrames();
    void renderFrame(double time, VfxPipe::VideoFrame outputFrame);

    WebVfx::QmlContent* content;
    QList<FrameSink> frameSinks;
    double duration;
    double initialTime;
    bool frameSwap;
};
