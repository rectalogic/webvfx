// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_QML_CONTENT_H_
#define WEBVFX_QML_CONTENT_H_

#include <QQmlError>
#include <QQuickView>
#include <QQuickRenderControl>
#include <QtQuick3D/QQuick3DTextureData>
#include "webvfx/content.h"
#include "webvfx/content_context.h"
#include "webvfx/effects.h"
#include "webvfx/image.h"

class QRhiRenderBuffer;
class QRhiRenderPassDescriptor;
class QRhiTexture;
class QRhiTextureRenderTarget;
class QSize;
class QUrl;

namespace WebVfx
{

class Image;
class Parameters;
class RenderStrategy;

class QmlContent : public QQuickView, public virtual Content
{
    Q_OBJECT
public:
    QmlContent(const QSize& size, Parameters* parameters);
    ~QmlContent();

    void loadContent(const QUrl& url) override;
    void setContentSize(const QSize& size) override;
    const Effects::ImageTypeMap& getImageTypeMap() override { return contentContext->getImageTypeMap(); };
    bool renderContent(double time, Image* renderImage) override;
    void setImage(const QString& name, Image* image) override { contentContext->setImage(name, image); }

signals:
    void contentLoadFinished(bool result);

private slots:
    bool initialize();
    void qmlViewStatusChanged(QQuickView::Status status);
    void logWarnings(const QList<QQmlError>& warnings);

private:
    QmlContent(QQuickRenderControl* renderControl, const QSize& size, Parameters* parameters);
    enum LoadStatus { LoadNotFinished, LoadFailed, LoadSucceeded };
    LoadStatus pageLoadFinished;
    ContentContext* contentContext;
    QQuickRenderControl* renderControl;

    QRhiTexture* texture;
    QRhiRenderBuffer* stencilBuffer;
    QRhiTextureRenderTarget* textureRenderTarget;
    QRhiRenderPassDescriptor* renderPassDescriptor;
    bool initialized;
};


class ImageTexture : public QQuick3DTextureData
{
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    QML_NAMED_ELEMENT(ImageTexture)

public:
    ImageTexture()
    {}

    QImage image() const
    {
        return m_image;
    }

public Q_SLOTS:
    void setImage(QImage image);

Q_SIGNALS:
    void imageChanged(QImage image);

private:
    void updateTexture();

    QImage m_image;
};

}

#endif
