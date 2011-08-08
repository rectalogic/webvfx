// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_RENDER_STRATEGY_H_
#define WEBVFX_RENDER_STRATEGY_H_

#include <QSize>

class QGLFramebufferObject;
class QGLWidget;

namespace WebVfx
{

class Content;
class Image;

class RenderStrategy
{
public:
    RenderStrategy() {}
    virtual ~RenderStrategy() {}
    virtual bool render(Content* content, Image* renderImage) = 0;
};

class GLWidgetRenderStrategy : public RenderStrategy
{
public:
    // Renderer does not take ownership of QGLWidget
    GLWidgetRenderStrategy(QGLWidget* glWidget);
    ~GLWidgetRenderStrategy();

    bool render(Content* content, Image* renderImage);

private:
    void createFBO(const QSize& size);

    QGLWidget* glWidget;
    QGLFramebufferObject* fbo;
};

class FBORenderStrategy : public RenderStrategy
{
public:
    // Renderer does not take ownership of QGLWidget
    FBORenderStrategy(QGLWidget* glWidget);
    ~FBORenderStrategy();

    bool render(Content* content, Image* renderImage);

private:
    void createFBOs(const QSize& size);

    QGLWidget* glWidget;
    QGLFramebufferObject* multisampleFBO;
    QGLFramebufferObject* resolveFBO;
};

class ImageRenderStrategy : public RenderStrategy
{
public:
    ImageRenderStrategy() {}

    bool render(Content* content, Image* renderImage);
};

}

#endif
