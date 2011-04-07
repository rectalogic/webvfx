// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include "browser.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationDomain("webvfx.org");
    app.setOrganizationName("WebVfx");
    app.setApplicationName("QtBrowser");

    Browser browser;
    browser.show();
    return app.exec();
}

