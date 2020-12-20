// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_CONTENT_CONTEXT_H_
#define WEBVFX_CONTENT_CONTEXT_H_

#include <QHash>
#include <QImage>
#include <QMap>
#include <QObject>
#include <QVariant>
#include <QUrl>
#include "webvfx/effects.h"

class QSize;
class QString;
typedef QMap<QString, QVariant> QVariantMap;

namespace WebVfx
{

class Image;
class Parameters;

// See QtWebKit Bridge docs:
// http://doc.qt.nokia.com/4.7-snapshot/qtwebkit-bridge.html

class ContentContext : public QObject
{
    Q_OBJECT
    //XXX We could use Q_ENUMS for this once this bug is fixed
    // http://bugreports.qt.nokia.com/browse/QTBUG-12706
    Q_PROPERTY(int SourceImageType READ getSourceImageType CONSTANT FINAL)
    Q_PROPERTY(int TargetImageType READ getTargetImageType CONSTANT FINAL)
    Q_PROPERTY(int ExtraImageType READ getExtraImageType CONSTANT FINAL)

public:
    ContentContext(QObject* parent, Parameters* parameters);
    ~ContentContext();

    // Inform page contents to render at time.
    // emits renderRequested signal to the page contents.
    void render(double time);

    // Set an image for the given name.
    // image must remain valid until render() is called.
    void setImage(const QString& name, Image* image);

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

    int getSourceImageType() { return Effects::SourceImageType; }
    int getTargetImageType() { return Effects::TargetImageType; }
    int getExtraImageType() { return Effects::ExtraImageType; }

    // Page contents should register if it consumes images.
    // JS:
    //   webvfx.registerImageType("video", webvfx.SourceImageType);
    Q_INVOKABLE void registerImageType(const QString& imageName, Effects::ImageType imageType);

    const Effects::ImageTypeMap& getImageTypeMap() { return imageTypeMap; }

signals:
    // Page contents must signal this when ready to render.
    // status indicates setup failure/success.
    void readyRender(bool status);

    // Signal raised when page contents should render for the given time.
    // time is normalized 0..1.0
    // JS: webvfx.renderRequested.connect(function (time) { doSomething(); })
    void renderRequested(double time);

private:
    Parameters* parameters;
    QHash<QString, QImage> imageMap;
    Effects::ImageTypeMap imageTypeMap;
    unsigned int renderCount;
};

}

#endif
