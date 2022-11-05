// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QImage>
#include <QObject>
#include <QSize>
#include <QStringList>

namespace WebVfx {
class QmlContent;
}

class FrameServer : public QObject {
    Q_OBJECT

public:
    FrameServer(const QSize& size, const QStringList& imageNames, const QMap<QString, QString>& propertyMap, const QUrl& qmlUrl, QObject* parent = nullptr);
    ~FrameServer();

    bool event(QEvent* event) override;

private slots:
    void onContentLoadFinished(bool);

private:
    void readFrames();
    void renderFrame();

    WebVfx::QmlContent* content;
    QSize videoSize;
    QStringList imageNames;
    unsigned int imageByteCount;
    unsigned int imageBufferReadSize;
    unsigned char* imageData;
    QImage* images;
};
