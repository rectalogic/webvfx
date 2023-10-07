// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QCommandLineParser> // for QCommandLineParser, QCommandLineParser::ParseAsLongOptions
#include <QCoreApplication> // for QCoreApplication
#include <QGuiApplication> // for QGuiApplication
#include <QList> // for QList
#include <QMessageLogContext> // for qCritical
#include <QStringList> // for QStringList
#include <QUrl> // for QUrl
#include <Qt> // for AA_ShareOpenGLContexts
#include <stdlib.h> // for exit
#ifdef WEBENGINEQUICK
#include <QtWebEngineQuick> // for initialize
#endif
#include "frameserver.h" // for FrameServer

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
