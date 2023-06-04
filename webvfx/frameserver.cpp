// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "frameserver.h"
#include "parameters.h"
#include "qml_content.h"
#include "webvfx.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <errno.h>
#include <unistd.h>
#include <vfxpipe.h>

class FrameServerParameters : public WebVfx::Parameters {
public:
    FrameServerParameters(QUrl& url)
        : urlQuery(url)
    {
        url.setQuery(QString());
    }

    double getNumberParameter(const QString& name)
    {
        return urlQuery.queryItemValue(name).toDouble();
    }

    QString getStringParameter(const QString& name)
    {
        return urlQuery.queryItemValue(name);
    }

private:
    QUrlQuery urlQuery;
};

/////////////////

FrameServer::FrameServer(QUrl& qmlUrl, double duration, QObject* parent)
    : QObject(parent)
    , content(new WebVfx::QmlContent(new FrameServerParameters(qmlUrl)))
    , duration(duration)
    , initialTime(-1)
{
    connect(content, &WebVfx::QmlContent::contentLoadFinished, this, &FrameServer::onContentLoadFinished);
    content->loadContent(qmlUrl);
}

FrameServer::~FrameServer()
{
    delete content;
}

void FrameServer::onContentLoadFinished(bool result)
{
    if (result) {
        auto size = content->getContentSize();
        auto videoSinks = content->getVideoSinks();
        for (qsizetype i = 0; i < videoSinks.size(); ++i) {
            frameSinks.append(FrameSink(videoSinks.at(i)));
        }
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    } else {
        qCritical() << "QML content failed to load.";
        QCoreApplication::exit(1);
        return;
    }
}

bool FrameServer::event(QEvent* event)
{
    if (event->type() == QEvent::User) {
        readFrames();
        return true;
    }
    return QObject::event(event);
}

void FrameServer::readFrames()
{
    auto f = __FUNCTION__;
    auto ioErrorHandler = [f](int n, std::string msg = "") {
        // EOF
        if (n == 0) {
            QCoreApplication::exit(0);
        } else if (n == -1) {
            qCritical() << f << ": WebVfx frameserver: " << msg.c_str();
            QCoreApplication::exit(1);
        }
    };

    double time;
    VfxPipe::dataIO(STDIN_FILENO, reinterpret_cast<uchar*>(&time), sizeof(time), read, ioErrorHandler);

    if (initialTime == -1) {
        initialTime = time;
    }
    time = time - initialTime;
    if (duration != 0) {
        time = time / duration;
    }

    VfxPipe::VideoFrame outputFrame;
    VfxPipe::readVideoFrame(STDIN_FILENO, &outputFrame, ioErrorHandler);
    content->setContentSize(QSize(outputFrame.format.width, outputFrame.format.height));

    uint32_t frameCount;
    VfxPipe::dataIO(STDIN_FILENO, reinterpret_cast<std::byte*>(&frameCount), sizeof(frameCount), read, ioErrorHandler);
    if (frameCount != frameSinks.size()) {
        qCritical() << "Frame count " << frameCount << " does not match video sink count " << frameSinks.size();
        QCoreApplication::exit(1);
    }
    for (qsizetype i = 0; i < frameSinks.size(); ++i) {
        VfxPipe::VideoFrame inputFrame;
        VfxPipe::readVideoFrame(STDIN_FILENO, &inputFrame, ioErrorHandler);
        auto frameSink = frameSinks.at(i);
        if (frameSink.format != inputFrame.format) {
            frameSink.format = inputFrame.format;
            if (inputFrame.format.pixelFormat == VfxPipe::VideoFrameFormat::PixelFormat::RGBA32) {
                QVideoFrameFormat format(QSize(inputFrame.format.width, inputFrame.format.height),
                    QVideoFrameFormat::PixelFormat::Format_ARGB8888_Premultiplied);
                frameSink.frames[0] = QVideoFrame(format);
                frameSink.frames[1] = QVideoFrame(format);
            } else {
                qCritical() << "Unrecognized video frame format " << inputFrame.format.pixelFormat;
                QCoreApplication::exit(1);
            }
        }
        QVideoFrame frame = frameSwap ? frameSink.frames[0] : frameSink.frames[1];
        frame.map(QVideoFrame::WriteOnly);
        if (frame.mappedBytes(0) != inputFrame.format.dataSize()) {
            qCritical() << "QVideoFrame incorrect byte size: " << frame.mappedBytes(0) << " expected " << inputFrame.format.dataSize();
            QCoreApplication::exit(1);
        }
        VfxPipe::dataIO(STDIN_FILENO, frame.bits(0), frame.mappedBytes(0), read, ioErrorHandler);
        frame.unmap();
        frameSink.sink->setVideoFrame(frame);
    }

    renderFrame(time, outputFrame);
    frameSwap = !frameSwap;
}

void FrameServer::renderFrame(double time, VfxPipe::VideoFrame outputFrame)
{
    auto ioErrorHandler = [](int n, std::string msg = "") {
        // EOF
        if (n == 0) {
            QCoreApplication::exit(0);
        } else if (n == -1) {
            qCritical() << msg.c_str();
            QCoreApplication::exit(1);
        }
    };
    // XXX need to convert the rendered RGBA32 image to outputFrame format
    QImage outputImage = content->renderContent(time);
    if (outputImage.isNull()) {
        qCritical() << "Null image rendered";
        QCoreApplication::exit(1);
    }
    VfxPipe::dataIO(STDOUT_FILENO, outputImage.constBits(), outputImage.sizeInBytes(), write, ioErrorHandler);
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}