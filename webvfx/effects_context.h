// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_EFFECTS_CONTEXT_H_
#define WEBVFX_EFFECTS_CONTEXT_H_

#include <QHash>
#include <QObject>
#include <QPixmap>
#include "webvfx/effects.h"

class QImage;
class QSize;
class QString;
class QVariant;
typedef QMap<QString, QVariant> QVariantMap;

namespace WebVfx
{

class Image;
class WebPage;
class Parameters;

// See QtWebKit Bridge docs:
// http://doc.qt.nokia.com/4.7-snapshot/qtwebkit-bridge.html

class EffectsContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int SOURCE_IMAGE_TYPE READ getSourceImageType CONSTANT FINAL)
    Q_PROPERTY(int TARGET_IMAGE_TYPE READ getTargetImageType CONSTANT FINAL)
    Q_PROPERTY(int EXTRA_IMAGE_TYPE READ getExtraImageType CONSTANT FINAL)
public:
    EffectsContext(WebPage* parent, Parameters* parameters);
    ~EffectsContext();

    // Inform page contents to render at time.
    // emits renderRequested signal to the page contents.
    void render(double time);

    // Get a Image of the specified size for writing image data to.
    // The returned image is only valid until the next call to getImage.
    Image getImage(const QString& name, const QSize& size);

    // Page contents can use these to retrieve parameters.
    // JS: var title = webvfx.getStringParameter("title");
    Q_INVOKABLE double getNumberParameter(const QString& name);
    Q_INVOKABLE const QString getStringParameter(const QString& name);

    // Page contents can retrieve named images.
    // JS:
    //   var image = new Image();
    //   webvfx.getImage("video").assignToHTMLElement(image);
    Q_INVOKABLE const QPixmap getImage(const QString& name);

    //XXX expose error signal - JS in page can raise on window.onerror? or use it to signal error during rendering


    int getSourceImageType() { return Effects::SourceImageType; }
    int getTargetImageType() { return Effects::TargetImageType; }
    int getExtraImageType() { return Effects::ExtraImageType; }

signals:
    // Page contents must signal this when load is complete
    // (which may be after window.onload fires).
    // status indicates load failure/success.
    // Map should map image names to their ImageType.
    // JS: webvfx.loadFinished(true, { "video" : webvfx.SOURCE_IMAGE_TYPE });
    void loadFinished(bool status, const QVariantMap& imageTypeMap);

    // Signal raised when page contents should render for the given time.
    // time is normalized 0..1.0
    // JS: webvfx.renderRequested.connect(function (time) { doSomething(); })
    void renderRequested(double time);

private:
    Parameters* parameters;
    QHash<QString, QImage*> imageMap;
};

}

#endif
