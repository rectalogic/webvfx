#include <QByteArray>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickImageProvider>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickView>
#include <QResizeEvent>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QtGui/private/qrhi_p.h>
#include <QtQuick/private/qquickrendercontrol_p.h>

#include "webvfx/qml_content.h"
#include "webvfx/webvfx.h"

namespace WebVfx {

class ImageProvider : public QQuickImageProvider {
public:
    ImageProvider(ContentContext* contentContext)
        : QQuickImageProvider(QQmlImageProviderBase::Image)
        , contentContext(contentContext)
    {
    }

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize)
    {
        // URLs are of the form image://webvfx/<name>/<count>
        // where <count> is a unique ID to force refresh and is ignored.
        QImage image(contentContext->getImage(id.section('/', 0, 0)));

        if (size)
            *size = image.size();

        if (!requestedSize.isEmpty())
            return image.scaled(requestedSize, Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation);

        return image;
    }

private:
    ContentContext* contentContext;
};

////////////////////

void ImageTexture::setImage(QImage image)
{
    updateTexture(image);
    emit imageChanged(m_image);
}

void ImageTexture::updateTexture(QImage image)
{
    setSize(image.size());
    bool needsConvert = false;
    switch (image.format()) {
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
        break;
    default:
        needsConvert = true;
        break;
    }
    setHasTransparency(image.hasAlphaChannel());
    setFormat(QQuick3DTextureData::RGBA8);
    if (needsConvert)
        m_image = image.convertToFormat(QImage::Format_RGBA8888);
    else if (image.width() * image.depth() != image.bytesPerLine() * 8)
        m_image = image.copy();
    else
        m_image = image;
    setTextureData(QByteArray::fromRawData((const char*)m_image.constBits(), m_image.sizeInBytes()));
}

////////////////////

QmlContent::QmlContent(QQuickRenderControl* renderControl, const QSize& size, Parameters* parameters)
    : QQuickView(QUrl(), renderControl)
    , pageLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters, size))
    , renderControl(renderControl)
    , texture(0)
    , stencilBuffer(0)
    , textureRenderTarget(0)
    , renderPassDescriptor(0)
    , initialized(false)
{
    // Add root of our qrc:/ resource path so embedded QML components are available.
    engine()->addImportPath(":/");

    setResizeMode(ResizeMode::SizeViewToRootObject);

    // Expose context to the QML
    rootContext()->setContextProperty("webvfx", contentContext);

    // Register image provider for image://webvfx/<name>/<counter>
    engine()->addImageProvider(QLatin1String("webvfx"), new ImageProvider(contentContext));

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
    delete renderControl;
}

void QmlContent::uninitialize()
{
    if (!initialized)
        return;
    delete renderPassDescriptor;
    renderPassDescriptor = 0;
    delete textureRenderTarget;
    textureRenderTarget = 0;
    delete stencilBuffer;
    stencilBuffer = 0;
    delete texture;
    texture = 0;
    renderControl->invalidate();
    initialized = false;
}

bool QmlContent::initialize()
{
    if (initialized)
        return true;
    if (!renderControl->initialize()) {
        log("Failed to initialize render control");
        return false;
    }

    QRhi* rhi = QQuickRenderControlPrivate::get(renderControl)->rhi;

    const QSize size = contentContext->getVideoSize();
    texture = rhi->newTexture(QRhiTexture::RGBA8, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
    if (!texture->create()) {
        log("Failed to create texture");
        return false;
    }

    // depth-stencil is mandatory with RHI, although strictly speaking the
    // scenegraph could operate without one, but it has no means to figure out
    // the lack of a ds buffer, so just be nice and provide one.
    stencilBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, size, 1);
    if (!stencilBuffer->create()) {
        log("Failed to create render buffer");
        return false;
    }

    QRhiTextureRenderTargetDescription renderTargetDescription((QRhiColorAttachment(texture)));
    renderTargetDescription.setDepthStencilBuffer(stencilBuffer);
    textureRenderTarget = rhi->newTextureRenderTarget(renderTargetDescription);
    renderPassDescriptor = textureRenderTarget->newCompatibleRenderPassDescriptor();
    textureRenderTarget->setRenderPassDescriptor(renderPassDescriptor);
    if (!textureRenderTarget->create()) {
        log("Failed to create render target");
        return false;
    }

    // redirect Qt Quick rendering into our texture
    setRenderTarget(QQuickRenderTarget::fromRhiRenderTarget(textureRenderTarget));

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
        log(warning.toString());
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
    QQuickRenderControlPrivate* renderControlPrivate = QQuickRenderControlPrivate::get(renderControl);
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
    readbackBatch->readBackTexture(texture, &readResult);
    renderControlPrivate->cb->resourceUpdate(readbackBatch);

    renderControl->endFrame();

    return readCompleted;
}

}