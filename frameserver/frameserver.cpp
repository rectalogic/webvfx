// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QDebug>
#include <QMap>
#include <QSize>
#include <QUrl>
#include "frameserver.h"
#include <webvfx/webvfx.h>
#include <webvfx/logger.h>
#include <webvfx/parameters.h>
#include <webvfx/qml_content.h>

class FrameServerParameters : public WebVfx::Parameters
{
public:
    FrameServerParameters(QMap<QString, QString> map) : propertyMap(map) {}

    double getNumberParameter(const QString& name) {
        return propertyMap[name].toDouble();
    }

    QString getStringParameter(const QString& name) {
        return propertyMap[name];
    }

private:
    QMap<QString, QString> propertyMap;
};

/////////////////

class FrameServerLogger : public WebVfx::Logger {
public:
    void log(const QString& message) {
        qDebug() << message;
    }
};

/////////////////

FrameServer::FrameServer(QObject *parent, const QSize &size, int frameCount, const QMap<QString, QString>& propertyMap, const QUrl& qmlUrl)
    : QObject(parent)
    , content(0)
{
    WebVfx::setLogger(new FrameServerLogger());
    content = new WebVfx::QmlContent(size, new FrameServerParameters(propertyMap));
    connect(content, SIGNAL(contentLoadFinished(bool)), SLOT(onContentLoadFinished(bool)));
    content->loadContent(qmlUrl);
}

FrameServer::~FrameServer()
{
    delete content;
}

void FrameServer::onContentLoadFinished(bool result)
{
    if (result) {
        //XXX build image map using named images, keep track of order so we can read into them each render
        //XXX fetch image spec from content so we know what we have
        //XXX allocate single chunk of memory for N images, wrap sections with Image/QImage
        //XXX then can just read() untill full then render
    }
    else {
        // XXX fail
    }
}
