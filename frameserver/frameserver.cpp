// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <errno.h>
#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QUrl>
#include "frameserver.h"
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
    , imageByteCount(videoSize.width() * videoSize.height() * 4)
    , imageBufferReadSize(0)
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
        // Single buffer to hold output image and all input images, plus timecode
        imageData = new unsigned char[sizeof(double) + ((1 + imageNames.size()) * imageByteCount)];
        images = new QImage[1 + imageNames.size()]; //XXX use QList/emplace_back ?
        for (int i = 0; i < imageNames.size(); i++) {
            images[i] = QImage((const uchar*)(imageData + sizeof(double) + (i * imageByteCount)),
                videoSize.width(), videoSize.height(), QImage::Format_RGBA8888);
            content->setImage(imageNames.at(i), images[i]);
        }
        // Last image is the output image
        images[imageNames.size()] = QImage((uchar*)(imageData + sizeof(double) + (imageNames.size() * imageByteCount)),
            videoSize.width(), videoSize.height(), QImage::Format_RGBA8888);

        imageBufferReadSize = sizeof(double) + (imageByteCount * imageNames.size());
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }
    else {
        qCritical("QML content failed to load.");
        QCoreApplication::exit(1);
        return;
    }
}

bool FrameServer::event(QEvent *event) {
    if (event->type() == QEvent::User) {
        readFrames();
        return true;
    }
    return QObject::event(event);
}

void FrameServer::readFrames() {
    unsigned int currentBufferPosition = 0;

    while (currentBufferPosition < imageBufferReadSize) {
        ssize_t n = read(STDIN_FILENO, imageData + currentBufferPosition, imageBufferReadSize - currentBufferPosition);
        if (n == -1) {
            qCritical("read failed: %s", strerror(errno));
            QCoreApplication::exit(1);
            return;
        }
        else if (n == 0) {
            QCoreApplication::exit(0);
            return;
        }
        else {
            currentBufferPosition += n;
        }
    }
    renderFrame();
}

void FrameServer::renderFrame() {
    double time = *reinterpret_cast<double *>(imageData);
    auto outputImage = images[imageNames.size()];
    content->renderContent(time, outputImage);

    size_t bytesWritten = 0;
    while (bytesWritten < imageByteCount) {
        ssize_t n = write(STDOUT_FILENO, outputImage.constBits() + bytesWritten, imageByteCount - bytesWritten);
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

    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}