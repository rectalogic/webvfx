// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QHash>
#include <QImage>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QSize>
#include <QUrl>
#include <QVariant>
#include <QVideoSink>

class QString;
typedef QMap<QString, QVariant> QVariantMap;

namespace WebVfx {

class Parameters;

class ContentContext : public QObject {
    Q_OBJECT

    Q_PROPERTY(QSize videoSize READ getVideoSize WRITE setVideoSize NOTIFY videoSizeChanged)

public:
    ContentContext(QObject* parent, Parameters* parameters);
    ~ContentContext() override;

    // Inform QML contents to render at time.
    // emits renderRequested signal to the page contents.
    void render(double time);

    // QML contents can use these to retrieve parameters.
    // JS: var title = webvfx.getStringParameter("title");
    Q_INVOKABLE double getNumberParameter(const QString& name);
    Q_INVOKABLE QString getStringParameter(const QString& name);

    // QML contents can register a video sink.
    // JS:
    //   webvfx.addVideoSink(output.videoSink);
    Q_INVOKABLE void addVideoSink(QVideoSink* videoSink);

    QSize getVideoSize() const { return videoSize; };
    void setVideoSize(QSize size);
    const QList<QVideoSink*>& getVideoSinks() { return videoSinks; }

signals:
    // Signal raised when QML contents should render for the given time.
    // time is normalized 0..1.0
    // JS: webvfx.renderRequested.connect(function (time) { doSomething(); })
    void renderRequested(double time);
    void videoSizeChanged(QSize size);

private:
    Parameters* parameters;
    QList<QVideoSink*> videoSinks;
    QSize videoSize;
};

}
