// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "content_context.h" // for ContentContext
#include <QList> // for QList
#include <QObject> // for Q_OBJECT, signals, slots
#include <QQuickView> // for QQuickView, QQuickView::Status
#include <QScopedPointer> // for QScopedPointer
#include <QSize> // for QSize
class QImage;
class QQmlError;
class QUrl;
class QVideoSink;
namespace WebVfx {
class Parameters;
}
namespace WebVfx {
class RenderControl;
}

namespace WebVfx {

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
