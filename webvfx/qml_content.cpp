#include <QQuickImageProvider>
#include <QQuickRenderControl>
#include <QQuickView>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QResizeEvent>
#include <QSize>
#include <QString>
#include <QVariant>
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

QmlContent::QmlContent(const QSize& size, Parameters* parameters)
    : renderControl(new QQuickRenderControl())
    , QQuickView(QUrl(), renderControl)
    , pageLoadFinished(LoadNotFinished)
    , contextLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters))
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

QmlContent::~QmlContent()
{
    delete renderControl;
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
    // Allow the content to render for this time
    contentContext->render(time);
    return renderStrategy->render(this, renderImage);
    //XXX also check errors() after each render()
}

}
