// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>
#include "ui_browser.h"

class Browser : public QMainWindow, private Ui::Browser
{
    Q_OBJECT

public:
    Browser(QWidget *parent = 0);

private slots:
    void on_urlEdit_returnPressed();
    void on_webView_urlChanged(const QUrl&);
};

#endif

