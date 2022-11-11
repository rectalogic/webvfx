// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QImage>
#include <QList>
#include <QObject>
#include <QSize>

class QVideoFrame;
class QVideoSink;

namespace WebVfx {
class QmlContent;
}

struct FrameSink {
    FrameSink(QVideoFrame* frame1, QVideoFrame* frame2, QVideoSink* sink)
        : sink(sink)
    {
        frames[0] = frame1;
        frames[1] = frame2;
    };
    QVideoFrame* frames[2];
    QVideoSink* sink;
};

class FrameServer : public QObject {
    Q_OBJECT

public:
    FrameServer(const QSize& size, const QMap<QString, QString>& propertyMap, const QUrl& qmlUrl, double duration = 0, QObject* parent = nullptr);
    ~FrameServer();

    bool event(QEvent* event) override;

private slots:
    void onContentLoadFinished(bool);

private:
    void readBytes(uchar* buffer, size_t bufferSize);
    void readFrames();
    void writeBytes(const uchar* buffer, size_t bufferSize);
    void renderFrame(double time);

    WebVfx::QmlContent* content;
    QList<FrameSink> frameSinks;
    QImage outputImage;
    double duration;
    double initialTime;
    bool frameSwap;
};
