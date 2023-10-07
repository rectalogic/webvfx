// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "viewer.h" // for Viewer
#include <QApplication> // for QApplication
#include <QList> // for QList
#include <QMessageLogContext> // for qInstallMessageHandler, QtMsgType
#include <QScopedPointer> // for QScopedPointer
#include <QString> // for QString
#include <QStringList> // for QStringList

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationDomain("webvfx.stream");
    app.setOrganizationName("WebVfx");
    app.setApplicationName("WebVfx Viewer");

    // Can't use a capturing lambda as a function pointer
    // https://stackoverflow.com/questions/73366933/what-does-mean-in-cpp-lambda-declaration-auto-fun1
    // Need to ensure viewer is destroyed before we leave main and exit,
    // otherwise it's destructor tries to clean up global state which is in the unloaded webvfx library

    struct Cleanup {
        static inline void cleanup(Viewer* pointer)
        {
            qInstallMessageHandler(0);
            delete pointer;
        }
    };
    static Viewer* viewer = new Viewer();
    QScopedPointer<Viewer, Cleanup> destroyer(viewer);

    auto messageHandler = +[](QtMsgType type, const QMessageLogContext& context, const QString& msg) mutable {
        viewer->messageHandler(type, context, msg);
    };
    qInstallMessageHandler(messageHandler);
    viewer->show();

    QStringList args(QApplication::arguments());
    if (args.size() > 1)
        viewer->loadFile(args.at(1));

    return app.exec();
}
