// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "frameserver.h"
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMap>
#include <QSize>
#include <QUrl>
#ifdef WEBENGINEQUICK
#include <QtWebEngineQuick>
#endif
#include <stdlib.h>

inline void assertOpt(bool r)
{
    if (!r) {
        qCritical("Failed to add option");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
#ifdef TARGET_OS_MAC
    putenv(const_cast<char*>("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM=1"));
#endif

#ifdef WEBENGINEQUICK
#if !defined(QT_NO_OPENGL)
    // https://doc.qt.io/qt-6/qml-qtwebengine-webengineview.html#rendering-to-opengl-surface
    // https://doc.qt.io/qt-6/qtwebengine-overview.html#embedding-web-content-into-qt-quick-applications
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif
    QtWebEngineQuick::initialize();
#endif

    QGuiApplication app(argc, argv);

    app.setOrganizationDomain("webvfx.stream");
    app.setOrganizationName("WebVfx");
    app.setApplicationName("WebVfx");

    QCommandLineParser parser;
    parser.setApplicationDescription("WebVfx frameserver");
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addPositionalArgument("source", "QML source URL.");

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        qCritical("Missing required source url");
        exit(1);
    }
    QUrl url(args.at(0));
    new FrameServer(url, &app);

    return app.exec();
}
