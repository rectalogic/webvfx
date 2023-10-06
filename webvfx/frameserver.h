// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QEvent>
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
    FrameSink(QList<QVideoSink*> sinks)
        : sinks(sinks) {};
    VfxPipe::VideoFrameFormat format;
    QVideoFrame frames[2];
    QList<QVideoSink*> sinks;
};

class FrameServer : public QObject {
    Q_OBJECT

public:
    FrameServer(QUrl& qmlUrl, QObject* parent = nullptr);
    ~FrameServer();

    bool event(QEvent* event) override;

private slots:
    void onContentLoadFinished(bool);
    void onRenderComplete(QImage);

private:
    void readFrames();

    WebVfx::QmlContent* content;
    QList<FrameSink> frameSinks;
    double duration;
    double initialTime;
    bool frameSwap;
    VfxPipe::RenderedVideoFrame outputFrame;
    static QEvent::Type renderEventType;
};
