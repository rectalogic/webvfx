// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include "viewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationDomain("webvfx.org");
    app.setOrganizationName("WebVfx");
    app.setApplicationName("WebVfx Viewer");

    Viewer viewer;
    viewer.show();

    QStringList args(QApplication::arguments());
    if (args.size() > 1)
        viewer.loadFile(args.at(1));

    return app.exec();
}

