// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_RENDERER_H_
#define WEBVFX_RENDERER_H_

#include <QSize>

class QGLFramebufferObject;
class QGLWidget;

namespace WebVfx
{

class Content;
class Image;

class Renderer
{
public:
    Renderer() {}
    virtual ~Renderer() = 0;
    virtual bool render(Content* content, Image* renderImage) = 0;
};

class GLRenderer : public Renderer
{
public:
    // Renderer does not take ownership of QGLWidget
    GLRenderer(QGLWidget* glWidget)
        : glWidget(glWidget) {}

    bool render(Content* content, Image* renderImage);

private:
    QGLWidget* glWidget;
};

class GLAntialiasRenderer : public Renderer
{
public:
    // Renderer does not take ownership of QGLWidget
    GLAntialiasRenderer(QGLWidget* glWidget)
        : glWidget(glWidget)
        , multisampleFBO(0)
        , resolveFBO(0) {}
    ~GLAntialiasRenderer();

    bool render(Content* content, Image* renderImage);

private:
    bool createFBOs(const QSize& size);

    QGLWidget* glWidget;
    QGLFramebufferObject* multisampleFBO;
    QGLFramebufferObject* resolveFBO;
};

class ImageRenderer : public Renderer
{
public:
    ImageRenderer() {}

    bool render(Content* content, Image* renderImage);
};

}

#endif
