// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "content_context.h"
#include <QImage>
#include <QQmlError>
#include <QQuickView>
#include <QScopedPointer>

class QSize;
class QVideoSink;
class QUrl;

namespace WebVfx {

class Parameters;
class RenderControl;

class QmlContent : public QQuickView {
    Q_OBJECT
public:
    QmlContent(Parameters* parameters);
    ~QmlContent() override;

    void loadContent(const QUrl& url);
    void setContentSize(const QSize& size);
    QSize getContentSize() { return contentContext->getVideoSize(); }
    const QList<QList<QVideoSink*>>& getVideoSinks() { return contentContext->getVideoSinks(); }
    void renderContent(double time);

signals:
    void contentLoadFinished(bool result);
    void renderComplete(QImage renderImage);

private slots:
    void qmlViewStatusChanged(QQuickView::Status status);
    void logWarnings(const QList<QQmlError>& warnings);
    void contextAsyncRenderComplete();

private:
    bool initialize();
    void uninitialize();

    QmlContent(RenderControl* renderControl, Parameters* parameters);
    ContentContext* contentContext;
    QScopedPointer<RenderControl> renderControl;
    bool renderExpected;
};

}
