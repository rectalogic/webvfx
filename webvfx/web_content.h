// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_WEB_CONTENT_H_
#define WEBVFX_WEB_CONTENT_H_

#include <QMap>

#include <QGraphicsView>
#include <QWebPage>
#include "webvfx/content.h"
#include "webvfx/content_context.h"
#include "webvfx/effects.h"

class QEventLoop;
class QGraphicsWebView;
class QImage;
class QSize;
class QWebFrame;

namespace WebVfx
{

class Image;
class Parameters;
class RenderStrategy;
class WebPage;

class WebContent : public QGraphicsView, public virtual Content
{
    Q_OBJECT
public:
    WebContent(const QSize& size, Parameters* parameters);
    ~WebContent();

    // Load URL synchronously, return success
    bool loadContent(const QUrl& url);
    void setContentSize(const QSize& size);
    const Effects::ImageTypeMap& getImageTypeMap() { return contentContext->getImageTypeMap(); };
    bool renderContent(double time, Image* renderImage);
    void paintContent(QPainter* painter);
    void setImage(const QString& name, Image* image) { contentContext->setImage(name, image); }

    QWidget* createView(QWidget* parent);

    // For debugging with Viewer
    QWebSettings* settings();

private slots:
    void injectContentContext();
    void webPageLoadFinished(bool result);
    void contentContextLoadFinished(bool result);

private:
    QGraphicsWebView* webView;
    WebPage* webPage;
    enum LoadStatus { LoadNotFinished, LoadFailed, LoadSucceeded };
    LoadStatus pageLoadFinished;
    LoadStatus contextLoadFinished;
    ContentContext* contentContext;
    QEventLoop* syncLoop;
    RenderStrategy* renderStrategy;
};

////////////////////

class QWebFrame;
class QNetworkRequest;

class WebPage : public QWebPage
{
    Q_OBJECT
public:
    WebPage(QObject* parent);

private slots:
    bool shouldInterruptJavaScript();

protected:
    void javaScriptAlert(QWebFrame *, const QString &msg);
    void javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID);
    bool acceptNavigationRequest(QWebFrame*, const QNetworkRequest&, NavigationType);
};

}

#endif
