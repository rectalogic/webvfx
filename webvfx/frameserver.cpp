// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "frameserver.h"
#include "parameters.h"
#include "qml_content.h"
#include "webvfx.h"
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
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
    FrameServerParameters(QUrlQuery& urlQuery)
        : urlQuery(urlQuery)
    {
    }

    double getNumberParameter(const QString& name)
    {
        return urlQuery.queryItemValue(name, QUrl::FullyDecoded).toDouble();
    }

    QString getStringParameter(const QString& name)
    {
        return urlQuery.queryItemValue(name, QUrl::FullyDecoded);
    }

private:
    QUrlQuery urlQuery;
};

double parseDuration(QString durationValue)
{
    double duration = 0;
    if (!durationValue.isEmpty()) {
        bool dOk;
        duration = durationValue.toDouble(&dOk);
        // Parse as rational
        if (!dOk) {
            bool error = true;
            QStringList parts = durationValue.split(QChar('/'));
            if (parts.size() == 2) {
                error = false;
                double numerator = parts.at(0).toDouble(&dOk);
                if (!dOk)
                    error = true;
                double denominator = parts.at(1).toDouble(&dOk);
                if (!dOk || denominator == 0)
                    error = true;
                if (!error)
                    duration = numerator / denominator;
            }
            if (error) {
                qCritical("Invalid duration.");
                exit(1);
            }
        }
    }
    return duration;
}

/////////////////

QEvent::Type FrameServer::renderEventType = static_cast<QEvent::Type>(QEvent::registerEventType());

FrameServer::FrameServer(QUrl& qmlUrl, QObject* parent)
    : QObject(parent)
    , duration(0)
    , initialTime(-1)
    , frameSwap(false)
{
    auto urlQuery = QUrlQuery(qmlUrl);
    duration = parseDuration(urlQuery.queryItemValue("webvfx_duration", QUrl::FullyDecoded));
    urlQuery.removeQueryItem("webvfx_duration");

    content = new WebVfx::QmlContent(new FrameServerParameters(urlQuery));
    qmlUrl.setQuery(QString());

    connect(content, &WebVfx::QmlContent::contentLoadFinished, this, &FrameServer::onContentLoadFinished);
    connect(content, &WebVfx::QmlContent::renderComplete, this, &FrameServer::onRenderComplete);
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
        QCoreApplication::postEvent(this, new QEvent(renderEventType));
    } else {
        qCritical() << "QML content failed to load.";
        QCoreApplication::exit(1);
        return;
    }
}

bool FrameServer::event(QEvent* event)
{
    if (event->type() == renderEventType) {
        readFrames();
        return true;
    }
    return QObject::event(event);
}

void FrameServer::readFrames()
{
    auto f = __FUNCTION__;
    auto ioErrorHandler = [f](std::string msg) {
        // EOF
        if (msg.empty()) {
            QCoreApplication::exit(0);
        } else {
            qCritical() << f << ": WebVfx frameserver: " << msg.c_str();
            QCoreApplication::exit(1);
        }
    };

    double time;
    if (!VfxPipe::dataIO(STDIN_FILENO, reinterpret_cast<uchar*>(&time), sizeof(time), read, ioErrorHandler))
        return;

    if (initialTime == -1) {
        initialTime = time;
    }
    time = time - initialTime;
    if (duration != 0) {
        time = time / duration;
    }

    if (!VfxPipe::readVideoFrameFormat(STDIN_FILENO, outputFrame.format, ioErrorHandler))
        return;
    content->setContentSize(QSize(outputFrame.format.width, outputFrame.format.height));

    uint32_t frameCount;
    if (!VfxPipe::dataIO(STDIN_FILENO, reinterpret_cast<std::byte*>(&frameCount), sizeof(frameCount), read, ioErrorHandler))
        return;
    if (frameCount > frameSinks.size()) {
        qCritical() << "Frame count " << frameCount << " does not match video sink count " << frameSinks.size();
        QCoreApplication::exit(1);
        return;
    }
    for (auto& frameSink : frameSinks) {
        VfxPipe::VideoFrame<std::byte*> inputFrame;
        if (!VfxPipe::readVideoFrameFormat(STDIN_FILENO, inputFrame.format, ioErrorHandler))
            return;
        if (frameSink.format != inputFrame.format) {
            frameSink.format = inputFrame.format;
            if (inputFrame.format.pixelFormat == VfxPipe::VideoFrameFormat::PixelFormat::RGBA32) {
                QVideoFrameFormat format(QSize(inputFrame.format.width, inputFrame.format.height),
                    QVideoFrameFormat::PixelFormat::Format_RGBA8888);
                frameSink.frames[0] = QVideoFrame(format);
                frameSink.frames[1] = QVideoFrame(format);
            } else {
                qCritical() << "Unrecognized video frame format " << inputFrame.format.pixelFormat;
                QCoreApplication::exit(1);
                return;
            }
        }
        QVideoFrame frame = frameSwap ? frameSink.frames[0] : frameSink.frames[1];
        frame.map(QVideoFrame::WriteOnly);
        if (frame.mappedBytes(0) != inputFrame.format.dataSize()) {
            qCritical() << "QVideoFrame incorrect byte size: " << frame.mappedBytes(0) << " expected " << inputFrame.format.dataSize();
            QCoreApplication::exit(1);
            return;
        }
        if (!VfxPipe::dataIO(STDIN_FILENO, frame.bits(0), frame.mappedBytes(0), read, ioErrorHandler))
            return;
        frame.unmap();
        for (const auto& sink : frameSink.sinks) {
            sink->setVideoFrame(frame);
        }
    }

    content->renderContent(time);
    frameSwap = !frameSwap;
}

void FrameServer::onRenderComplete(QImage outputImage)
{
    // XXX need to convert the rendered RGBA32 outputImage to outputFrame format

    auto ioErrorHandler = [](std::string msg) {
        // EOF
        if (msg.empty()) {
            QCoreApplication::exit(0);
        } else {
            qCritical() << msg.c_str();
            QCoreApplication::exit(1);
        }
    };

    if (outputImage.isNull()) {
        qCritical() << "Null image rendered";
        QCoreApplication::exit(1);
        return;
    }
    if (!VfxPipe::dataIO(STDOUT_FILENO, outputImage.constBits(), outputImage.sizeInBytes(), write, ioErrorHandler))
        return;
    QCoreApplication::postEvent(this, new QEvent(renderEventType));
}
