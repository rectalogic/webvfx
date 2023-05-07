// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_QML_CONTENT_H_
#define WEBVFX_QML_CONTENT_H_

#include "webvfx/content_context.h"
#include <QImage>
#include <QQmlError>
#include <QQuickRenderControl>
#include <QQuickView>
#include <QScopedPointer>
#include <QSet>

class QRhiRenderBuffer;
class QRhiRenderPassDescriptor;
class QRhiTexture;
class QRhiTextureRenderTarget;
class QSize;
class QVideoSink;
class QUrl;

namespace WebVfx {

class Parameters;

class QmlContent : public QQuickView {
    Q_OBJECT
public:
    QmlContent(const QSize& size, Parameters* parameters);
    ~QmlContent() override;

    void loadContent(const QUrl& url);
    void setContentSize(const QSize& size);
    QSize getContentSize() { return contentContext->getVideoSize(); }
    const QList<QVideoSink*>& getVideoSinks() { return contentContext->getVideoSinks(); }
    bool renderContent(double time, QImage& renderImage);

signals:
    void contentLoadFinished(bool result);

private slots:
    void qmlViewStatusChanged(QQuickView::Status status);
    void logWarnings(const QList<QQmlError>& warnings);

private:
    bool initialize();
    void uninitialize();

    QmlContent(QQuickRenderControl* renderControl, const QSize& size, Parameters* parameters);
    enum LoadStatus { LoadNotFinished,
        LoadFailed,
        LoadSucceeded };
    LoadStatus pageLoadFinished;
    ContentContext* contentContext;
    QScopedPointer<QQuickRenderControl> renderControl;

    QScopedPointer<QRhiTexture> texture;
    QScopedPointer<QRhiRenderBuffer> stencilBuffer;
    QScopedPointer<QRhiTextureRenderTarget> textureRenderTarget;
    QScopedPointer<QRhiRenderPassDescriptor> renderPassDescriptor;
    bool initialized;
};

}

#endif
