// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_QML_VIEW_H_
#define WEBVFX_QML_VIEW_H_

#include <QDeclarativeView>
#include <QGraphicsEffect>
#include "webvfx/content.h"
#include "webvfx/content_context.h"
#include "webvfx/effects.h"
#include "webvfx/image.h"

class QEventLoop;
class QGLFramebufferObject;
class QGLWidget;
class QSize;
class QUrl;

namespace WebVfx
{

class Image;
class Parameters;

class QmlContent : public QDeclarativeView, public Content
{
    Q_OBJECT
public:
    QmlContent(QWidget* parent, const QSize& size, Parameters* parameters);
    ~QmlContent();

    // Load QML synchronously, return success
    bool loadContent(const QUrl& url);
    void setContentSize(const QSize& size) { resize(size); }
    const Effects::ImageTypeMap& getImageTypeMap() { return contentContext->getImageTypeMap(); };
    Image renderContent(double time);
    Image getImage(const QString& name, const QSize& size) { return contentContext->getImage(name, size); }

private slots:
    void qmlViewStatusChanged(QDeclarativeView::Status status);
    void contentContextLoadFinished(bool result);
    void logWarnings(const QList<QDeclarativeError>& warnings);

private:
    bool createFBO(const QSize& size);

    enum LoadStatus { LoadNotFinished, LoadFailed, LoadSucceeded };
    LoadStatus pageLoadFinished;
    LoadStatus contextLoadFinished;
    ContentContext* contentContext;
    QEventLoop* syncLoop;
    QImage* renderImage;
    QGLWidget* glWidget;
    QGLFramebufferObject* multisampleFBO;
    QGLFramebufferObject* resolveFBO;
};

////////////////////

// QGraphicsEffect that captures its source as a pixmap.
// It does not render the source on-screen, just captures to a pixmap property.
class GraphicsCaptureEffect : public QGraphicsEffect
{
    Q_OBJECT
    Q_PROPERTY(QPixmap pixmap READ pixmap NOTIFY pixmapChanged)
public:
    GraphicsCaptureEffect(QObject* parent=0) : QGraphicsEffect(parent) {}
    ~GraphicsCaptureEffect() {}

    QPixmap pixmap() const { return capturedPixmap; }

Q_SIGNALS:
    void pixmapChanged(const QPixmap& pixmap);

protected:
    void draw(QPainter*)
    {
        QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates);
        if (capturedPixmap.cacheKey() != pixmap.cacheKey()) {
            capturedPixmap = pixmap;
            emit pixmapChanged(capturedPixmap);
        }
    }

private:
    QPixmap capturedPixmap;
};

}

#endif
