// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_QML_CONTENT_H_
#define WEBVFX_QML_CONTENT_H_

#include <QDeclarativeView>
#include <QGraphicsEffect>
#include "webvfx/content.h"
#include "webvfx/content_context.h"
#include "webvfx/effects.h"
#include "webvfx/image.h"

class QSize;
class QUrl;

namespace WebVfx
{

class Image;
class Parameters;
class RenderStrategy;

class QmlContent : public QDeclarativeView, public virtual Content
{
    Q_OBJECT
public:
    QmlContent(const QSize& size, Parameters* parameters);
    ~QmlContent();

    void loadContent(const QUrl& url);
    void setContentSize(const QSize& size);
    const Effects::ImageTypeMap& getImageTypeMap() { return contentContext->getImageTypeMap(); };
    bool renderContent(double time, Image* renderImage);
    void paintContent(QPainter* painter);
    void setImage(const QString& name, Image* image) { contentContext->setImage(name, image); }

    QWidget* createView(QWidget* parent) {
        setParent(parent);
        return this;
    }

signals:
    void contentLoadFinished(bool result);
    void contentPreLoadFinished(bool result);

private slots:
    void qmlViewStatusChanged(QDeclarativeView::Status status);
    void contentContextLoadFinished(bool result);
    void logWarnings(const QList<QDeclarativeError>& warnings);

private:
    enum LoadStatus { LoadNotFinished, LoadFailed, LoadSucceeded };
    LoadStatus pageLoadFinished;
    LoadStatus contextLoadFinished;
    ContentContext* contentContext;
    RenderStrategy* renderStrategy;
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
    void sourceChanged(ChangeFlags) {
        updateCapture();
    }
    void draw(QPainter*) {
        updateCapture();
    }
private:
    void updateCapture() {
        QPixmap pixmap = sourcePixmap();
        if (capturedPixmap.cacheKey() != pixmap.cacheKey()) {
            capturedPixmap = pixmap;
            emit pixmapChanged(capturedPixmap);
        }
    }

    QPixmap capturedPixmap;
};

}

#endif
