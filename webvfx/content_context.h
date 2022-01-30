// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_CONTENT_CONTEXT_H_
#define WEBVFX_CONTENT_CONTEXT_H_

#include <QHash>
#include <QImage>
#include <QMap>
#include <QSet>
#include <QObject>
#include <QVariant>
#include <QUrl>

class QSize;
class QString;
typedef QMap<QString, QVariant> QVariantMap;

namespace WebVfx
{

class Parameters;

// See QtWebKit Bridge docs:
// http://doc.qt.nokia.com/4.7-snapshot/qtwebkit-bridge.html

class ContentContext : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int videoWidth READ getVideoWidth NOTIFY videoSizeChanged)
    Q_PROPERTY(int videoHeight READ getVideoHeight NOTIFY videoSizeChanged)

public:
    ContentContext(QObject* parent, Parameters* parameters, QSize size);
    ~ContentContext() override;

    // Inform page contents to render at time.
    // emits renderRequested signal to the page contents.
    void render(double time);

    // Set an image for the given name.
    // image must remain valid until render() is called.
    void setImage(const QString& name, QImage image);

    // Page contents can use these to retrieve parameters.
    // JS: var title = webvfx.getStringParameter("title");
    Q_INVOKABLE double getNumberParameter(const QString& name);
    Q_INVOKABLE QString getStringParameter(const QString& name);

    // Page contents can retrieve named images.
    // JS:
    //   var image = new Image();
    //   webvfx.getImage("video").assignToHTMLElement(image);
    Q_INVOKABLE QImage getImage(const QString& name);

    // Return URL for use in QML to reference the named image
    Q_INVOKABLE QUrl getImageUrl(const QString& name);

    int getVideoWidth() { return videoSize.width(); }
    int getVideoHeight() { return videoSize.height(); }
    QSize getVideoSize() { return videoSize; };
    void setVideoSize(QSize size);

    // Page contents should register if it consumes images.
    // JS:
    //   webvfx.registerImageName("video");
    Q_INVOKABLE void registerImageName(const QString& imageName);

    const QSet<QString>& getImageNames() { return imageNameSet; }

signals:
    // Signal raised when page contents should render for the given time.
    // time is normalized 0..1.0
    // JS: webvfx.renderRequested.connect(function (time) { doSomething(); })
    void renderRequested(double time);
    void videoSizeChanged();

private:
    Parameters* parameters;
    QHash<QString, QImage> imageMap;
    QSet<QString> imageNameSet;
    unsigned int renderCount;
    QSize videoSize;
};

}

#endif
