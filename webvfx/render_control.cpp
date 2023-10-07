#include "render_control.h"
#include <QByteArray> // for QByteArray
#include <QDebug> // for QDebug
#include <QImage> // for QImage, QImage::Format_RGBA8888_Premultiplied
#include <QQuickRenderControl> // for QQuickRenderControl
#include <QQuickRenderTarget> // for QQuickRenderTarget
#include <QQuickWindow> // for QQuickWindow
#include <QSize> // for QSize, operator==
#include <QtGlobal> // for qDebug
#include <QtGui/private/qrhi_p.h> // for QRhiTextureRenderTarget, QRhiTexture, QRhi, QRhiReadbackResult, QRhiRenderBuffer, QRhiRenderPassDescriptor, operator|, QRhiColorAttachment, QRhiTextureRenderTargetDescription, QRhiCommandBuffer, QRhiRenderBuffer::DepthStencil, QRhiResour...
#include <QtQuick/private/qquickrendercontrol_p.h> // for QQuickRenderControlPrivate
#include <QtTypes> // for uchar
#include <functional> // for function

namespace WebVfx {

RenderControl::RenderControl()
    : QQuickRenderControl()
    , initialized(false)
{
}

RenderControl::~RenderControl()
{
}

bool RenderControl::install(QQuickWindow* window, QSize size)
{
    if (initialized && size == renderSize)
        return true;

    if (!initialized) {
        if (!initialize()) {
            qDebug() << "Failed to initialize render control";
            return false;
        }
        initialized = true;
    }

    window->setRenderTarget(QQuickRenderTarget());
    textureRenderTarget.reset();
    renderPassDescriptor.reset();
    stencilBuffer.reset();
    texture.reset();

    QRhi* rhi = QQuickRenderControlPrivate::get(this)->rhi;

    texture.reset(rhi->newTexture(QRhiTexture::RGBA8, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!texture->create()) {
        qDebug() << "Failed to create texture";
        return false;
    }

    // depth-stencil is mandatory with RHI, although strictly speaking the
    // scenegraph could operate without one, but it has no means to figure out
    // the lack of a ds buffer, so just be nice and provide one.
    stencilBuffer.reset(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, size, 1));
    if (!stencilBuffer->create()) {
        qDebug() << "Failed to create render buffer";
        return false;
    }

    QRhiTextureRenderTargetDescription renderTargetDescription((QRhiColorAttachment(texture.data())));
    renderTargetDescription.setDepthStencilBuffer(stencilBuffer.data());
    textureRenderTarget.reset(rhi->newTextureRenderTarget(renderTargetDescription));
    renderPassDescriptor.reset(textureRenderTarget->newCompatibleRenderPassDescriptor());
    textureRenderTarget->setRenderPassDescriptor(renderPassDescriptor.data());
    if (!textureRenderTarget->create()) {
        qDebug() << "Failed to create render target";
        return false;
    }

    auto renderTarget = QQuickRenderTarget::fromRhiRenderTarget(textureRenderTarget.data());
    if (rhi->isYUpInFramebuffer())
        renderTarget.setMirrorVertically(true);

    // redirect Qt Quick rendering into our texture
    window->setRenderTarget(renderTarget);

    renderSize = size;
    return true;
}

QImage RenderControl::renderImage()
{
    polishItems();
    beginFrame();
    sync();
    render();

    // RHI is private, we need to use it to avoid writing platform specific code for every platform
    // See https://bugreports.qt.io/browse/QTBUG-88876
    QQuickRenderControlPrivate* renderControlPrivate = QQuickRenderControlPrivate::get(this);
    QRhi* rhi = renderControlPrivate->rhi;

    QImage outputImage;
    QRhiReadbackResult readResult;
    readResult.completed = [&readResult, &rhi, &outputImage] {
        QByteArray* imageData = new QByteArray(readResult.data);
        QImage sourceImage(
            reinterpret_cast<const uchar*>(imageData->constData()),
            readResult.pixelSize.width(), readResult.pixelSize.height(),
            QImage::Format_RGBA8888_Premultiplied,
            [](void* ptr) {
                delete static_cast<QByteArray*>(ptr);
            },
            imageData);
        outputImage = sourceImage;
    };
    QRhiResourceUpdateBatch* readbackBatch = rhi->nextResourceUpdateBatch();
    readbackBatch->readBackTexture(texture.data(), &readResult);
    renderControlPrivate->cb->resourceUpdate(readbackBatch);

    endFrame();

    return outputImage;
}

}
