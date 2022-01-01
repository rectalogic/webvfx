// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QMap>
#include <QSize>
#include "frameserver.h"

inline void assertOpt(bool r) {
    if (!r) {
        qFatal("Failed to add option");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setOrganizationDomain("webvfx.org");
    app.setOrganizationName("WebVfx");
    app.setApplicationName("WebVfx");

    QCommandLineParser parser;
    parser.setApplicationDescription("WebVfx render pipe");
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    assertOpt(parser.addOption({{"p", "property"}, "Property name=value.", "property"}));
    assertOpt(parser.addOption({"width", "Video frame width.", "width"}));
    assertOpt(parser.addOption({"height", "Video frame height.", "height"}));
    assertOpt(parser.addOption({{"f", "frames"}, "Number of frames to be rendered.", "frameCount"}));
    parser.addPositionalArgument("source", "QML source URL.");

    parser.process(app);

    QMap<QString, QString> propertyMap;
    const auto properties = parser.values("property");
    for (int i = 0; i < properties.size(); ++i) {
        const auto value = properties.at(i);
        propertyMap[value.section('=', 0, 0)] = value.section('=', 1, -1, QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);
    }

    const auto widthValue = parser.value("width");
    const auto heightValue = parser.value("height");
    bool wOk, hOk;
    const int width = widthValue.toInt(&wOk);
    const int height = heightValue.toInt(&hOk);
    if (!wOk || !hOk) {
        qFatal("Invalid frame size.");
        exit(1);
    }

    const auto frameCountValue = parser.value("frames");
    bool fOk;
    const int frameCount = frameCountValue.toInt(&fOk);
    if (!fOk) {
        qFatal("Invalid frame count.");
        exit(1);
    }

    const QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        qFatal("Missing required source url");
        exit(1);
    }
    const QUrl url(args.at(0));

    const FrameServer frameserver(&app, QSize(width, height), frameCount, propertyMap, url);

    return app.exec();
}

