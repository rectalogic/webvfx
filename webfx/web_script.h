// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_SCRIPT_H_
#define WEBFX_WEB_SCRIPT_H_

#include <QObject>

namespace WebFX
{

class WebPage;
class WebParameters;

// See QtWebKit Bridge
// http://doc.qt.nokia.com/4.7-snapshot/qtwebkit-bridge.html

class WebScript : public QObject
{
    Q_OBJECT
public:
    WebScript(WebPage* parent, WebParameters* parameters);
    ~WebScript();

    void render(double time);

    // Loaded page can use these to retrieve parameters.
    // JS: var title = webfx.getStringParameter("title");
    Q_INVOKABLE double getNumberParameter(const QString& name);
    Q_INVOKABLE const QString getStringParameter(const QString& name);

    //XXX expose error slot - JS in page can connect window.onerror to it? or use it to signal error during rendering

signals:
    // Loaded page must signal this when load is complete
    // (which may be after window.onload fires).
    // status indicates load failure/success.
    // JS: webfx.loadFinished(true);
    void loadFinished(bool status);

    // Signal raised when loaded page should render for the given time.
    // time is normalized 0..1.0
    // JS: webfx.renderRequested.connect(function (time) { doSomething(); })
    void renderRequested(double time);

private:
    WebParameters* parameters;
};

}

#endif
