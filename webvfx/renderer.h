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
    enum RenderType { RenderGL, RenderGLAntialias, RenderNoGL };

    Renderer()
        : glWidget(0)
        , renderType(RenderGL)
        , multisampleFBO(0)
        , resolveFBO(0) {}
    ~Renderer();

    void init(QGLWidget* glWidget, const QSize& size);
    void setRenderType(RenderType type);
    bool render(Content* content, Image* renderImage);
    void resize(const QSize& size);

private:
    void deleteFBOs();
    bool createFBOs();
    bool renderGL(Content* content, Image* renderImage);
    bool renderGLAntialias(Content* content, Image* renderImage);
    bool renderNoGL(Content* content, Image* renderImage);

    QGLWidget* glWidget;
    RenderType renderType;
    QGLFramebufferObject* multisampleFBO;
    QGLFramebufferObject* resolveFBO;
    QSize size;
};

}

#endif
