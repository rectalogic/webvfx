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

FrameServer::FrameServer(const QSize& size, QUrl& qmlUrl, double duration, QObject* parent)
    : QObject(parent)
    , content(new WebVfx::QmlContent(size, new FrameServerParameters(qmlUrl)))
    , outputImage(size, QImage::Format_RGBA8888)
    , duration(duration)
    , initialTime(-1)
{
    connect(content, &WebVfx::QmlContent::contentLoadFinished, this, &FrameServer::onContentLoadFinished);
    content->loadContent(qmlUrl);
}

FrameServer::~FrameServer()
{
    for (qsizetype i = 0; i < frameSinks.size(); ++i) {
        auto frameSink = frameSinks.at(i);
        delete frameSink.frames[0];
        delete frameSink.frames[1];
    }
    delete content;
}

void FrameServer::onContentLoadFinished(bool result)
{
    if (result) {
        auto size = content->getContentSize();
        auto videoSinks = content->getVideoSinks();
        for (qsizetype i = 0; i < videoSinks.size(); ++i) {
            frameSinks.append(FrameSink(
                new QVideoFrame(QVideoFrameFormat(size, QVideoFrameFormat::Format_RGBA8888)),
                new QVideoFrame(QVideoFrameFormat(size, QVideoFrameFormat::Format_RGBA8888)),
                videoSinks.at(i)));
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
    auto ioErrorHandler = [](int n, std::string msg = "") {
        // EOF
        if (n == 0) {
            QCoreApplication::exit(0);
        } else if (n == -1) {
            qCritical() << msg.c_str();
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

    for (qsizetype i = 0; i < frameSinks.size(); ++i) {
        auto frameSink = frameSinks.at(i);
        QVideoFrame* frame = frameSwap ? frameSink.frames[0] : frameSink.frames[1];
        frame->map(QVideoFrame::WriteOnly);
        VfxPipe::dataIO(STDIN_FILENO, frame->bits(0), frame->mappedBytes(0), read, ioErrorHandler);
        frame->unmap();
        frameSink.sink->setVideoFrame(*frame);
    }

    renderFrame(time);
    frameSwap = !frameSwap;
}

void FrameServer::renderFrame(double time)
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
    content->renderContent(time, outputImage);
    VfxPipe::dataIO(STDOUT_FILENO, outputImage.constBits(), outputImage.sizeInBytes(), write, ioErrorHandler);
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}