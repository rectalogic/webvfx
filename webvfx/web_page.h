// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_WEB_PAGE_H_
#define WEBVFX_WEB_PAGE_H_

#include <string>
#include <QWebPage>
#include "webvfx/web_effects.h"
#include "webvfx/web_image.h"
#include "webvfx/web_script.h"

class QEventLoop;
class QImage;
class QSize;
class QWebFrame;

namespace WebVFX
{

class WebParameters;

class WebPage : public QWebPage
{
    Q_OBJECT
public:
    WebPage(QObject* parent, QSize size, WebParameters* parameters);
    ~WebPage();

    // Load URL synchronously, return success
    bool loadSync(const QUrl& url);
    const WebEffects::ImageTypeMap& getImageTypeMap() { return imageTypeMap; };
    WebImage render(double time);
    WebImage getWebImage(const QString& name, const QSize& size) { return webScript->getWebImage(name, size); }

private slots:
    void injectWebScript();
    bool shouldInterruptJavaScript();
    void webPageLoadFinished(bool result);
    void webScriptLoadFinished(bool result, const QVariantMap& imageTypeMap);

protected:
    void javaScriptAlert(QWebFrame* originatingFrame, const QString& msg);
    void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
    bool acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest& request, NavigationType type);

private:
    enum LoadStatus { LoadNotFinished, LoadFailed, LoadSucceeded };
    LoadStatus pageLoadFinished;
    LoadStatus scriptLoadFinished;
    WebScript* webScript;
    QEventLoop* syncLoop;
    QImage* renderImage;
    WebEffects::ImageTypeMap imageTypeMap;
};

}

#endif
