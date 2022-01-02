// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QObject>
#include <QSize>
#include <QStringList>
#include <webvfx/image.h>

namespace WebVfx
{
    class QmlContent;
}

class FrameServer : public QObject
{
    Q_OBJECT

public:
    FrameServer(const QSize &size, unsigned int frameCount, const QStringList& imageNames, const QMap<QString, QString>& propertyMap, const QUrl& qmlUrl, QObject *parent = nullptr);
    ~FrameServer();

private slots:
    void onContentLoadFinished(bool);

private:
    void renderFrame();

    WebVfx::QmlContent* content;
    QSize videoSize;
    unsigned int frameCount;
    unsigned int currentFrame;
    QStringList imageNames;
    unsigned int imageByteCount;
    unsigned char *imageData;
    WebVfx::Image *images;
};
