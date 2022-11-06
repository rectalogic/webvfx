// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "frameserver.h"
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMap>
#include <QSize>
#include <QUrl>

inline void assertOpt(bool r)
{
    if (!r) {
        qCritical("Failed to add option");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    app.setOrganizationDomain("webvfx.org");
    app.setOrganizationName("WebVfx");
    app.setApplicationName("WebVfx");

    QCommandLineParser parser;
    parser.setApplicationDescription("WebVfx render pipe");
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    assertOpt(parser.addOption({ { "p", "property" }, "Property name=value, may be specified multiple times.", "property" }));
    assertOpt(parser.addOption({ "width", "Video frame width.", "width" }));
    assertOpt(parser.addOption({ "height", "Video frame height.", "height" }));
    assertOpt(parser.addOption({ "duration", "Video duration in seconds (floating point or rational e.g. 199/30).", "duration" }));
    assertOpt(parser.addOption({ { "i", "image" }, "Name of image on stdin, may be specified multiple times, order matters.", "image" }));
    parser.addPositionalArgument("source", "QML source URL.");

    parser.process(app);

    const auto imageNames = parser.values("image");

    QMap<QString, QString> propertyMap;
    const auto properties = parser.values("property");
    for (int i = 0; i < properties.size(); ++i) {
        const auto value = properties.at(i);
        propertyMap[value.section('=', 0, 0)] = value.section('=', 1, -1, QString::SectionIncludeTrailingSep);
    }

    const auto widthValue = parser.value("width");
    const auto heightValue = parser.value("height");
    bool wOk, hOk;
    const int width = widthValue.toInt(&wOk);
    const int height = heightValue.toInt(&hOk);
    if (!wOk || !hOk) {
        qCritical("Invalid frame size.");
        exit(1);
    }

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
    const QUrl url(args.at(0));

    new FrameServer(QSize(width, height), imageNames, propertyMap, url, duration, &app);

    return app.exec();
}
