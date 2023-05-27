#include <QByteArray>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickView>
#include <QResizeEvent>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QtDebug>
#include <QtGui/private/qrhi_p.h>
#include <QtQuick/private/qquickrendercontrol_p.h>

#include "qml_content.h"
#include "webvfx.h"

namespace WebVfx {

QmlContent::QmlContent(QQuickRenderControl* renderControl, const QSize& size, Parameters* parameters)
    : QQuickView(QUrl(), renderControl)
    , pageLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters, size))
    , renderControl(renderControl)
    , initialized(false)
{
    setResizeMode(ResizeMode::SizeViewToRootObject);

    // Expose context to the QML
    rootContext()->setContextProperty("webvfx", contentContext);

    connect(this, SIGNAL(statusChanged(QQuickView::Status)), SLOT(qmlViewStatusChanged(QQuickView::Status)));
    connect(engine(), SIGNAL(warnings(QList<QQmlError>)), SLOT(logWarnings(QList<QQmlError>)));
}

QmlContent::QmlContent(const QSize& size, Parameters* parameters)
    : QmlContent(new QQuickRenderControl(), size, parameters)
{
}

QmlContent::~QmlContent()
{
    uninitialize();
}

void QmlContent::uninitialize()
{
    if (!initialized)
        return;
    renderPassDescriptor.reset();
    textureRenderTarget.reset();
    stencilBuffer.reset();
    texture.reset();
    renderControl->invalidate();
    initialized = false;
}

bool QmlContent::initialize()
{
    if (initialized)
        return true;
    if (!renderControl->initialize()) {
        qDebug() << "Failed to initialize render control";
        return false;
    }

    QRhi* rhi = QQuickRenderControlPrivate::get(renderControl.data())->rhi;

    const QSize size = contentContext->getVideoSize();
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

    // redirect Qt Quick rendering into our texture
    setRenderTarget(QQuickRenderTarget::fromRhiRenderTarget(textureRenderTarget.data()));

    initialized = true;
    return true;
}

void QmlContent::qmlViewStatusChanged(QQuickView::Status status)
{
    if (status != QQuickView::Ready && status != QQuickView::Error)
        return;

    if (pageLoadFinished == LoadNotFinished)
        pageLoadFinished = (status == QQuickView::Ready) ? LoadSucceeded : LoadFailed;

    logWarnings(errors());
    emit contentLoadFinished(pageLoadFinished == LoadSucceeded);
}

void QmlContent::logWarnings(const QList<QQmlError>& warnings)
{
    foreach (const QQmlError& warning, warnings) {
        qDebug() << warning.toString();
    }
}

void QmlContent::loadContent(const QUrl& url)
{
    pageLoadFinished = LoadNotFinished;
    setSource(url);
}

void QmlContent::setContentSize(const QSize& size)
{
    if (contentContext->getVideoSize() != size) {
        contentContext->setVideoSize(size);
        uninitialize();
    }
}

bool QmlContent::renderContent(double time, QImage& renderImage)
{
    if (!initialize()) {
        return false;
    }
    // Allow the content to render for this time
    contentContext->render(time);

    renderControl->polishItems();
    renderControl->beginFrame();
    renderControl->sync();
    renderControl->render();

    if (status() == QQuickView::Error) {
        logWarnings(errors());
    }

    // RHI is private, we need to use it to avoid writing platform specific code for every platform
    // See https://bugreports.qt.io/browse/QTBUG-88876
    QQuickRenderControlPrivate* renderControlPrivate = QQuickRenderControlPrivate::get(renderControl.data());
    QRhi* rhi = renderControlPrivate->rhi;

    bool readCompleted = false;
    QRhiReadbackResult readResult;
    readResult.completed = [&readCompleted, &readResult, &rhi, &renderImage] {
        readCompleted = true;
        QImage sourceImage(reinterpret_cast<const uchar*>(readResult.data.constData()),
            readResult.pixelSize.width(), readResult.pixelSize.height(),
            QImage::Format_RGBA8888_Premultiplied);
        QPainter painter(&renderImage);
        if (rhi->isYUpInFramebuffer()) {
            painter.scale(1, -1);
            painter.translate(0, -readResult.pixelSize.height());
        }
        painter.drawImage(0, 0, sourceImage);
        painter.end();
    };
    QRhiResourceUpdateBatch* readbackBatch = rhi->nextResourceUpdateBatch();
    readbackBatch->readBackTexture(texture.data(), &readResult);
    renderControlPrivate->cb->resourceUpdate(readbackBatch);

    renderControl->endFrame();

    return readCompleted;
}

}