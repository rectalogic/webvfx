// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QUrl>
#include <QWebView>
#include "browser.h"


Browser::Browser(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    QWebSettings *settings = webView->settings();
    settings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    settings->setAttribute(QWebSettings::SiteSpecificQuirksEnabled, false);
    settings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
    settings->setAttribute(QWebSettings::WebGLEnabled, true);
#endif
}

void Browser::on_urlEdit_returnPressed()
{
    webView->load(QUrl::fromUserInput(urlEdit->text()));
}

void Browser::on_webView_urlChanged(const QUrl& url)
{
    urlEdit->setText(url.toString());
}

