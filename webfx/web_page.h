// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_WEB_PAGE_H_
#define CHROMIX_WEB_PAGE_H_

#include <QWebPage>

class QWebFrame;
class QEventLoop;

namespace Chromix
{

class WebPage : public QWebPage
{
    Q_OBJECT
public:
    WebPage(QObject* parent = 0);

    // Load URL synchronously, return success
    bool loadSync(const QUrl& url);

private slots:
    bool shouldInterruptJavaScript();
    void loadSyncFinished(bool result);

protected:
    void javaScriptAlert(QWebFrame* originatingFrame, const QString& msg);
    void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);

private:
    QEventLoop* syncLoop;
};

}

#endif
