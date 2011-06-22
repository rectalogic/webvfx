// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_RENDERER_H_
#define WEBVFX_RENDERER_H_

class QGLFramebufferObject;
class QGLWidget;
class QSize;

namespace WebVfx
{

class Content;
class Image;

class Renderer
{
public:
    //XXX make antialiasing optional
    Renderer();
    ~Renderer();

    bool init(QGLWidget* glWidget, bool ownWidget, const QSize& size);
    bool render(Content* content, Image* renderImage);
    bool resize(const QSize& size);

private:
    QGLWidget* glWidget;
    bool ownWidget;
    QGLFramebufferObject* multisampleFBO;
    QGLFramebufferObject* resolveFBO;
};

}

#endif
