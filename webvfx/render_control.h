// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QQuickRenderControl>
#include <QScopedPointer>
#include <QSize>

class QQuickWindow;
class QRhiTexture;
class QRhiRenderBuffer;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;

namespace WebVfx {

class RenderControl : public QQuickRenderControl {
    Q_OBJECT
public:
    RenderControl();
    ~RenderControl();

    bool install(QQuickWindow* window, QSize size);
    QImage renderImage();
    void invalidate();

private:
    QScopedPointer<QRhiTexture> texture;
    QScopedPointer<QRhiRenderBuffer> stencilBuffer;
    QScopedPointer<QRhiTextureRenderTarget> textureRenderTarget;
    QScopedPointer<QRhiRenderPassDescriptor> renderPassDescriptor;
    QSize renderSize;
};

}
