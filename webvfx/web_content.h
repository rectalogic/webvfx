// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_WEB_CONTENT_H_
#define WEBVFX_WEB_CONTENT_H_

#include <QMap>
#include <QWebPage>
#include "webvfx/content.h"
#include "webvfx/content_context.h"
#include "webvfx/effects.h"

class QEventLoop;
class QImage;
class QSize;
class QWebFrame;

namespace WebVfx
{

class Image;
class Parameters;

class WebContent : public QWebPage, public Content
{
    Q_OBJECT
public:
    WebContent(QObject* parent, const QSize& size, Parameters* parameters);
    ~WebContent();

    // Load URL synchronously, return success
    bool loadContent(const QUrl& url);
    void setContentSize(const QSize& size) {
        if (viewportSize() != size)
            setViewportSize(size);
    }
    const Effects::ImageTypeMap& getImageTypeMap() { return contentContext->getImageTypeMap(); };
    bool renderContent(double time, Image* renderImage);
    void setImage(const QString& name, Image* image) { contentContext->setImage(name, image); }

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
};

}

#endif
