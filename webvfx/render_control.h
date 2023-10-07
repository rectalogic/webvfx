// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QImage> // for QImage
#include <QObject> // for Q_OBJECT
#include <QQuickRenderControl> // for QQuickRenderControl
#include <QScopedPointer> // for QScopedPointer
#include <QSize> // for QSize
class QQuickWindow;
class QRhiRenderBuffer;
class QRhiRenderPassDescriptor;
class QRhiTexture;
class QRhiTextureRenderTarget;

namespace WebVfx {

class RenderControl : public QQuickRenderControl {
    Q_OBJECT
public:
    RenderControl();
    ~RenderControl();

    bool install(QQuickWindow* window, QSize size);
    QImage renderImage();

private:
    QScopedPointer<QRhiTexture> texture;
    QScopedPointer<QRhiRenderBuffer> stencilBuffer;
    QScopedPointer<QRhiTextureRenderTarget> textureRenderTarget;
    QScopedPointer<QRhiRenderPassDescriptor> renderPassDescriptor;
    QSize renderSize;
    bool initialized;
};

}
