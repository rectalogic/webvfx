// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_RENDERER_H_
#define WEBFX_WEB_RENDERER_H_

#include <QtWebKit>
#include "webfx/web_effects.h"
#include "webfx/web_image.h"

namespace WebFX
{

class WebImage;
class WebPage;
class WebParameters;

class WebRenderer : public QObject, public WebEffects
{
    Q_OBJECT
public:
    WebRenderer();
    ~WebRenderer() {};

    bool initialize(const std::string& url, int width, int height, WebParameters* parameters = 0);
    WebImage render(double time, int width, int height);
    void destroy();

private:
    Q_INVOKABLE void initializeInvokable(const QUrl& url, const QSize& size, WebParameters* parameters);
    Q_INVOKABLE void renderInvokable(double time, const QSize& size);

    // Test if we are currently on the UI thread
    bool onUIThread();

    WebPage* webPage;
    WebImage renderImage;
};

}

#endif
