// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_WEB_RENDERER_H_
#define WEBVFX_WEB_RENDERER_H_

#include <QObject>
#include <QSize>
#include <QUrl>
#include "webvfx/web_effects.h"

namespace WebVFX
{

class Image;
class WebPage;
class Parameters;

class WebRenderer : public QObject, public WebEffects
{
    Q_OBJECT
public:
    WebRenderer();
    ~WebRenderer() {};

    bool initialize(const std::string& url, int width, int height, Parameters* parameters = 0);
    const ImageTypeMap& getImageTypeMap();
    Image getImage(const std::string& name, int width, int height);
    const Image render(double time, int width, int height);
    void destroy();

private:
    Q_INVOKABLE void initializeInvokable(const QUrl& url, const QSize& size, Parameters* parameters);
    Q_INVOKABLE void renderInvokable(double time, const QSize& size);

    // Test if we are currently on the UI thread
    bool onUIThread();

    WebPage* webPage;
    Image renderImage;
    bool loadResult;
};

}

#endif
