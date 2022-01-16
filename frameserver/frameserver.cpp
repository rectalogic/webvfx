// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QUrl>
#include "frameserver.h"
#include "pipe_reader.h"
#include "common/webvfx_common.h"
#include <webvfx/webvfx.h>
#include <webvfx/logger.h>
#include <webvfx/parameters.h>
#include <webvfx/qml_content.h>

class FrameServerParameters : public WebVfx::Parameters
{
public:
    FrameServerParameters(QMap<QString, QString> map) : propertyMap(map) {}

    double getNumberParameter(const QString& name) {
        return propertyMap[name].toDouble();
    }

    QString getStringParameter(const QString& name) {
        return propertyMap[name];
    }

private:
    QMap<QString, QString> propertyMap;
};

/////////////////

class FrameServerLogger : public WebVfx::Logger {
public:
    void log(const QString& message) {
        qDebug() << message;
    }
};

/////////////////

FrameServer::FrameServer(const QSize &size, const QStringList& imageNames, const QMap<QString, QString>& propertyMap, const QUrl& qmlUrl, QObject *parent)
    : QObject(parent)
    , content(0)
    , videoSize(size)
    , imageNames(imageNames)
    , imageByteCount(videoSize.width() * videoSize.height() * WebVfxCommon::BytesPerPixel)
    , imageData(0)
    , images(0)
{
    WebVfx::setLogger(new FrameServerLogger());
    content = new WebVfx::QmlContent(size, new FrameServerParameters(propertyMap));
    connect(content, &WebVfx::QmlContent::contentLoadFinished, this, &FrameServer::onContentLoadFinished);

    content->loadContent(qmlUrl);
}

FrameServer::~FrameServer()
{
    delete[] images;
    delete[] imageData;
    delete content;
}

void FrameServer::onContentLoadFinished(bool result)
{
    if (result) {
        int imageCount = 1 + imageNames.size();
        // Single buffer to hold output image and all input images, plus timecode
        imageData = new unsigned char[sizeof(uint32_t) + (imageCount * imageByteCount)];
        images = new WebVfx::Image[imageCount];
        for (int i = 0; i < imageCount; i++) {
            images[i] = WebVfx::Image(imageData + sizeof(uint32_t) + (i * imageByteCount), videoSize.width(), videoSize.height(), imageByteCount);
            // Last image is the output image
            if (i < imageCount - 1)
                content->setImage(imageNames.at(i), &images[i]);
        }

        const auto readNotifier = new PipeReader(STDIN_FILENO, imageData, sizeof(uint32_t) + imageByteCount * imageNames.size(), this);
        connect(readNotifier, &PipeReader::bufferFilled, this, &FrameServer::renderFrame);
    }
    else {
        qCritical("QML content failed to load.");
        QCoreApplication::exit(1);
    }
}

void FrameServer::renderFrame() {
    //XXX producer needs to exit after last frame
    auto outputImage = &images[imageNames.size()];
    double time = WebVfxCommon::fromTimecode((uint32_t)*imageData);
    content->renderContent(time, outputImage);

    size_t bytesWritten = 0;
    while (bytesWritten < imageByteCount) {
        ssize_t n = write(STDOUT_FILENO, outputImage->pixels() + bytesWritten, imageByteCount - bytesWritten);
        // EOF
        if (n == 0) {
            QCoreApplication::exit(0);
        }
        if (n == -1) {
            qCritical("QML content failed to load.");
            QCoreApplication::exit(1);
        }
        bytesWritten = bytesWritten + n;
    }
}