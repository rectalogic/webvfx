// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_WEB_CONTENT_H_
#define WEBVFX_WEB_CONTENT_H_

#include <QMap>
#include <QWebPage>
#include "webvfx/content.h"
#include "webvfx/content_context.h"
#include "webvfx/image.h"
#include "webvfx/effects.h"

class QEventLoop;
class QImage;
class QSize;
class QWebFrame;

namespace WebVfx
{

class Parameters;

class WebContent : public QWebPage, public Content
{
    Q_OBJECT
public:
    WebContent(QObject* parent, QSize size, Parameters* parameters);
    ~WebContent();

    // Load URL synchronously, return success
    bool loadContent(const QUrl& url);
    void setContentSize(const QSize& size) { setViewportSize(size); }
    const Effects::ImageTypeMap& getImageTypeMap() { return contentContext->getImageTypeMap(); };
    Image renderContent(double time);
    Image getImage(const QString& name, const QSize& size) { return contentContext->getImage(name, size); }

private slots:
    void injectContentContext();
    bool shouldInterruptJavaScript();
    void webPageLoadFinished(bool result);
    void contentContextLoadFinished(bool result);

protected:
    void javaScriptAlert(QWebFrame* originatingFrame, const QString& msg);
    void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
    bool acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest& request, NavigationType type);

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
