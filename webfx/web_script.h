// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_SCRIPT_H_
#define WEBFX_WEB_SCRIPT_H_

#include <QHash>
#include <QObject>
#include <QPixmap>

class QImage;
class QSize;

namespace WebFX
{

class WebImage;
class WebPage;
class WebParameters;

// See QtWebKit Bridge docs:
// http://doc.qt.nokia.com/4.7-snapshot/qtwebkit-bridge.html

class WebScript : public QObject
{
    Q_OBJECT
public:
    WebScript(WebPage* parent, WebParameters* parameters);
    ~WebScript();

    // Inform page contents to render at time.
    // emits renderRequested signal to the page contents.
    void render(double time);

    // Get a WebImage of the specified size for writing image data to.
    // The returned image is only valid until the next call to getWebImage.
    WebImage getWebImage(const QString& name, const QSize& size);

    // Page contents can use these to retrieve parameters.
    // JS: var title = webfx.getStringParameter("title");
    Q_INVOKABLE double getNumberParameter(const QString& name);
    Q_INVOKABLE const QString getStringParameter(const QString& name);

    // Page contents can retrieve named images.
    // JS:
    //   var image = new Image();
    //   webfx.getImage("video").assignToHTMLElement(image);
    Q_INVOKABLE const QPixmap getImage(const QString& name);

    //XXX expose error slot - JS in page can connect window.onerror to it? or use it to signal error during rendering

signals:
    // Page contents must signal this when load is complete
    // (which may be after window.onload fires).
    // status indicates load failure/success.
    // JS: webfx.loadFinished(true);
    void loadFinished(bool status);

    // Signal raised when page contents should render for the given time.
    // time is normalized 0..1.0
    // JS: webfx.renderRequested.connect(function (time) { doSomething(); })
    void renderRequested(double time);

private:
    WebParameters* parameters;
    QHash<QString, QImage*> imageMap;
};

}

#endif
