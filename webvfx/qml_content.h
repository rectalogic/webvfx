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

    QmlContent(RenderControl* renderControl, const QSize& size, Parameters* parameters);
    enum LoadStatus { LoadNotFinished,
        LoadFailed,
        LoadSucceeded };
    LoadStatus pageLoadFinished;
    ContentContext* contentContext;
    QScopedPointer<RenderControl> renderControl;
};

}
