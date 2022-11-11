// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "frameserver.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <errno.h>
#include <unistd.h>
#include <webvfx/logger.h>
#include <webvfx/parameters.h>
#include <webvfx/qml_content.h>
#include <webvfx/webvfx.h>

class FrameServerParameters : public WebVfx::Parameters {
public:
    FrameServerParameters(QMap<QString, QString> map)
        : propertyMap(map)
    {
    }

    double getNumberParameter(const QString& name)
    {
        return propertyMap[name].toDouble();
    }

    QString getStringParameter(const QString& name)
    {
        return propertyMap[name];
    }

private:
    QMap<QString, QString> propertyMap;
};

/////////////////

class FrameServerLogger : public WebVfx::Logger {
public:
    void log(const QString& message)
    {
        qDebug() << message;
    }
};

/////////////////

class MutableVideoFrame : public QVideoFrame {
    using QVideoFrame::QVideoFrame;
    bool operator!=(const QVideoFrame& other) const { return true; }
    bool operator==(const QVideoFrame& other) const { return false; }
};

/////////////////

FrameServer::FrameServer(const QSize& size, const QMap<QString, QString>& propertyMap, const QUrl& qmlUrl, double duration, QObject* parent)
    : QObject(parent)
    , content(new WebVfx::QmlContent(size, new FrameServerParameters(propertyMap)))
    , outputImage(size, QImage::Format_RGBA8888)
    , duration(duration)
    , initialTime(-1)
{
    WebVfx::setLogger(new FrameServerLogger());
    connect(content, &WebVfx::QmlContent::contentLoadFinished, this, &FrameServer::onContentLoadFinished);
    content->loadContent(qmlUrl);
}

FrameServer::~FrameServer()
{
    for (qsizetype i = 0; i < frameSinks.size(); ++i) {
        delete frameSinks.at(i).frame;
    }
    delete content;
}

void FrameServer::onContentLoadFinished(bool result)
{
    if (result) {
        auto size = content->getContentSize();
        auto videoSinks = content->getVideoSinks();
        for (qsizetype i = 0; i < videoSinks.size(); ++i) {
            frameSinks.append(FrameSink(new MutableVideoFrame(QVideoFrameFormat(size, QVideoFrameFormat::Format_RGBA8888)), videoSinks.at(i)));
        }
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    } else {
        qCritical("QML content failed to load.");
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

void FrameServer::readBytes(uchar* buffer, size_t bufferSize)
{
    unsigned int currentBufferPosition = 0;

    while (currentBufferPosition < bufferSize) {
        ssize_t n = read(STDIN_FILENO, buffer + currentBufferPosition, bufferSize - currentBufferPosition);
        if (n == -1) {
            qCritical("read failed: %s", strerror(errno));
            QCoreApplication::exit(1);
            return;
        } else if (n == 0) {
            QCoreApplication::exit(0);
            return;
        } else {
            currentBufferPosition += n;
        }
    }
}

void FrameServer::readFrames()
{
    double time;
    readBytes(reinterpret_cast<uchar*>(&time), sizeof(time));
    if (initialTime == -1) {
        initialTime = time;
    }
    time = time - initialTime;
    if (duration != 0) {
        time = time / duration;
    }

    for (qsizetype i = 0; i < frameSinks.size(); ++i) {
        auto frameSink = frameSinks.at(i);
        frameSink.frame->map(QVideoFrame::WriteOnly);
        readBytes(frameSink.frame->bits(0), frameSink.frame->mappedBytes(0));
        frameSink.frame->unmap();
        frameSink.sink->setVideoFrame(*frameSink.frame);
    }

    renderFrame(time);
}

void FrameServer::writeBytes(const uchar* buffer, size_t bufferSize)
{
    size_t bytesWritten = 0;
    while (bytesWritten < bufferSize) {
        ssize_t n = write(STDOUT_FILENO, buffer + bytesWritten, bufferSize - bytesWritten);
        // EOF
        if (n == 0) {
            QCoreApplication::exit(0);
            return;
        }
        if (n == -1) {
            qCritical("QML content failed to load.");
            QCoreApplication::exit(1);
            return;
        }
        bytesWritten = bytesWritten + n;
    }
}

void FrameServer::renderFrame(double time)
{
    content->renderContent(time, outputImage);
    writeBytes(outputImage.constBits(), outputImage.sizeInBytes());
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}