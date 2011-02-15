// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_QML_VIEW_H_
#define WEBVFX_QML_VIEW_H_

#include <QDeclarativeView>
#include "webvfx/content.h"
#include "webvfx/content_context.h"
#include "webvfx/effects.h"
#include "webvfx/image.h"

class QEventLoop;
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
    QmlContent(QWidget* parent, QSize size, Parameters* parameters);
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
    enum LoadStatus { LoadNotFinished, LoadFailed, LoadSucceeded };
    LoadStatus pageLoadFinished;
    LoadStatus contextLoadFinished;
    ContentContext* contentContext;
    QEventLoop* syncLoop;
    QImage* renderImage;
};

}

#endif
