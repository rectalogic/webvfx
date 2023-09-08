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

    app.setOrganizationDomain("webvfx.org");
    app.setOrganizationName("WebVfx");
    app.setApplicationName("WebVfx");

    QCommandLineParser parser;
    parser.setApplicationDescription("WebVfx frameserver");
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    assertOpt(parser.addOption({ "duration", "Video duration in seconds (floating point or rational e.g. 199/30).", "duration" }));
    parser.addPositionalArgument("source", "QML source URL.");

    parser.process(app);

    double duration = 0;
    const auto durationValue = parser.value("duration");
    if (!durationValue.isEmpty()) {
        bool dOk;
        duration = durationValue.toDouble(&dOk);
        // Parse as rational
        if (!dOk) {
            bool error = true;
            QStringList parts = durationValue.split(QChar('/'));
            if (parts.size() == 2) {
                error = false;
                double numerator = parts.at(0).toDouble(&dOk);
                if (!dOk)
                    error = true;
                double denominator = parts.at(1).toDouble(&dOk);
                if (!dOk || denominator == 0)
                    error = true;
                if (!error)
                    duration = numerator / denominator;
            }
            if (error) {
                qCritical("Invalid duration.");
                exit(1);
            }
        }
    }

    const QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        qCritical("Missing required source url");
        exit(1);
    }
    QUrl url(args.at(0));

    new FrameServer(url, duration, &app);

    return app.exec();
}
