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
#include <QSet>
#include <QtQuick3D/QQuick3DTextureData>

class QRhiRenderBuffer;
class QRhiRenderPassDescriptor;
class QRhiTexture;
class QRhiTextureRenderTarget;
class QSize;
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
    const QSet<QString>& getImageNames() { return contentContext->getImageNames(); };
    bool renderContent(double time, QImage& renderImage);
    void setImage(const QString& name, QImage image) { contentContext->setImage(name, image); }

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
    QQuickRenderControl* renderControl;

    QRhiTexture* texture;
    QRhiRenderBuffer* stencilBuffer;
    QRhiTextureRenderTarget* textureRenderTarget;
    QRhiRenderPassDescriptor* renderPassDescriptor;
    bool initialized;
};

class ImageTexture : public QQuick3DTextureData {
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    QML_NAMED_ELEMENT(ImageTexture)

public:
    ImageTexture()
    {
    }

    QImage image() const
    {
        return m_image;
    }

public Q_SLOTS:
    void setImage(QImage image);

Q_SIGNALS:
    void imageChanged(QImage image);

private:
    void updateTexture(QImage image);

    QImage m_image;
};

}

#endif
