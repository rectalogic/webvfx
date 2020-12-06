#include <QQuickImageProvider>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickView>
#include <QImage>
#include <QList>
#include <QQmlError>
#include <QQmlContext>
#include <QResizeEvent>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QtGui/private/qrhi_p.h>
#include <QtQuick/private/qquickrendercontrol_p.h>

#include "webvfx/image.h"
#include "webvfx/qml_content.h"
#include "webvfx/webvfx.h"


namespace WebVfx
{

class ImageProvider : public QQuickImageProvider
{
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

QmlContent::QmlContent(QQuickRenderControl* renderControl, const QSize& size, Parameters* parameters)
    : QQuickView(QUrl(), renderControl)
    , pageLoadFinished(LoadNotFinished)
    , contextLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters))
    , renderControl(renderControl)
{
    // Add root of our qrc:/ resource path so embedded QML components are available.
    engine()->addImportPath(":/");

    setResizeMode(ResizeMode::SizeRootObjectToView);
    resize(size);

    // Expose context to the QML
    rootContext()->setContextProperty("webvfx", contentContext);

    // Register image provider for image://webvfx/<name>/<counter>
    engine()->addImageProvider(QLatin1String("webvfx"), new ImageProvider(contentContext));

    connect(this, SIGNAL(statusChanged(QQuickView::Status)), SLOT(qmlViewStatusChanged(QQuickView::Status)));
    connect(engine(), SIGNAL(warnings(QList<QDeclarativeError>)), SLOT(logWarnings(QList<QDeclarativeError>)));
    connect(contentContext, SIGNAL(readyRender(bool)), SLOT(contentContextLoadFinished(bool)));
}

QmlContent::QmlContent(const QSize& size, Parameters* parameters)
    : QmlContent(new QQuickRenderControl(), size, parameters)
{
}

QmlContent::~QmlContent()
{
    delete renderControl;

    //XXX these probably need to be deleted on SG thread?
    delete renderPassDescriptor;
    delete textureRenderTarget;
    delete stencilBuffer;
    delete texture;
}

bool QmlContent::initialize()
{
    if (!renderControl->initialize()) {
        log("Failed to initialize render control");
        return false;
    }

    QRhi *rhi = QQuickRenderControlPrivate::get(renderControl)->rhi;

    const QSize size = rootObject()->size().toSize();
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
    QRhiTextureRenderTarget* textureRenderTarget = rhi->newTextureRenderTarget(renderTargetDescription);
    QRhiRenderPassDescriptor* renderPassDescriptor = textureRenderTarget->newCompatibleRenderPassDescriptor();
    textureRenderTarget->setRenderPassDescriptor(renderPassDescriptor);
    if (!textureRenderTarget->create()) {
        log("Failed to create render target");
        return false;
    }

    // redirect Qt Quick rendering into our texture
    setRenderTarget(QQuickRenderTarget::fromRhiRenderTarget(textureRenderTarget));

    return true;
}

void QmlContent::qmlViewStatusChanged(QQuickView::Status status)
{
    if (status != QQuickView::Ready && status != QQuickView::Error)
        return;

    if (pageLoadFinished == LoadNotFinished)
        pageLoadFinished = (status == QQuickView::Ready) ? LoadSucceeded : LoadFailed;

    // This is useful when webvfx.renderReady(true) is not used.
    emit contentPreLoadFinished(pageLoadFinished == LoadSucceeded);

    if (pageLoadFinished == LoadFailed || contextLoadFinished != LoadNotFinished) {
        logWarnings(errors());
        emit contentLoadFinished(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
    }
}

void QmlContent::contentContextLoadFinished(bool result)
{
    if (contextLoadFinished == LoadNotFinished)
        contextLoadFinished = result ? LoadSucceeded : LoadFailed;
    if (contextLoadFinished == LoadFailed || pageLoadFinished != LoadNotFinished) {
        logWarnings(errors());
        emit contentLoadFinished(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
    }
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
    contextLoadFinished = LoadNotFinished;
    if (!initialize()) {
        pageLoadFinished = LoadFailed;
        emit contentPreLoadFinished(false);
        return;
    }
    setSource(url);
}

void QmlContent::setContentSize(const QSize& size) {
    QSize oldSize(this->size());
    if (oldSize != size) {
        resize(size);
        // The resize event is delayed until we are shown.
        // Since we are never shown, send the event here.
        // Superclass does some calculations in resizeEvent
        // (sizing and centering the scene etc.)
        QResizeEvent event(size, oldSize);
        resizeEvent(&event);
    }
}

bool QmlContent::renderContent(double time, Image* renderImage)
{
    if (!renderImage) {
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

    QQuickRenderControlPrivate *renderControlPrivate = QQuickRenderControlPrivate::get(renderControl);
    QRhi *rhi = renderControlPrivate->rhi;

    bool readCompleted = false;
    QRhiReadbackResult readResult;
    QImage result;
    readResult.completed = [&readCompleted, &readResult, &result, &rhi] {
        readCompleted = true;
        QImage wrapperImage(reinterpret_cast<const uchar *>(readResult.data.constData()),
                            readResult.pixelSize.width(), readResult.pixelSize.height(),
                            QImage::Format_RGBA8888_Premultiplied);
        if (rhi->isYUpInFramebuffer())
            result = wrapperImage.mirrored();
        else
            result = wrapperImage.copy();
    };
    QRhiResourceUpdateBatch *readbackBatch = rhi->nextResourceUpdateBatch();
    readbackBatch->readBackTexture(texture, &readResult);
    renderControlPrivate->cb->resourceUpdate(readbackBatch);

    renderControl->endFrame();

    return true;
}

}