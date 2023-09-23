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
    // QML contents can set this if it requires async rendering.
    // It should invoke emitAsyncRenderComplete when ready
    Q_PROPERTY(bool asyncRenderRequired READ isAsyncRenderRequired WRITE setAsyncRenderRequired)

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

    // QML contents can register a video source.
    // JS:
    //   int webvfx.addVideoSource();
    Q_INVOKABLE qsizetype addVideoSource();

    // QML contents can register a video sink.
    // JS:
    //   webvfx.appendVideoSink(source, output.videoSink);
    Q_INVOKABLE void appendVideoSink(qsizetype source, QVideoSink* videoSink);

    // QML contents should invoke this if it set asyncRenderRequired
    // JS:
    //   webvfx.emitAsyncRenderComplete()
    Q_INVOKABLE void emitAsyncRenderComplete();

    QSize getVideoSize() const { return videoSize; };
    void setVideoSize(QSize size);
    bool isAsyncRenderRequired() { return asyncRenderRequired; };
    void setAsyncRenderRequired(bool asyncRender) { asyncRenderRequired = asyncRender; };
    const QList<QList<QVideoSink*>>& getVideoSinks() { return videoSinks; }

signals:
    // Signal raised when QML contents should render for the given time.
    // time is normalized 0..1.0
    // JS: webvfx.renderRequested.connect(function (time) { doSomething(); })
    void renderRequested(double time);
    void videoSizeChanged(QSize size);

    void asyncRenderComplete();

private:
    Parameters* parameters;
    QList<QList<QVideoSink*>> videoSinks;
    QSize videoSize;
    bool asyncRenderRequired;
};

}
