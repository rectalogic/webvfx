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
    virtual ~Renderer() {}
    virtual bool render(Content* content, Image* renderImage) = 0;
};

class GLRenderer : public Renderer
{
public:
    // Renderer does not take ownership of QGLWidget
    GLRenderer(QGLWidget* glWidget);
    ~GLRenderer();

    bool render(Content* content, Image* renderImage);

private:
    void createFBO(const QSize& size);

    QGLWidget* glWidget;
    QGLFramebufferObject* fbo;
};

class GLAntialiasRenderer : public Renderer
{
public:
    // Renderer does not take ownership of QGLWidget
    GLAntialiasRenderer(QGLWidget* glWidget);
    ~GLAntialiasRenderer();

    bool render(Content* content, Image* renderImage);

private:
    void createFBOs(const QSize& size);

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
