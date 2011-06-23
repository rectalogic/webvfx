#include <QtDeclarative>
#include <QEventLoop>
#include <QGLWidget>
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

static bool s_QmlContentRegistered = false;

class PixmapProvider : public QDeclarativeImageProvider
{
 public:
     PixmapProvider(ContentContext* contentContext)
         : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
         , contentContext(contentContext)
     {
     }

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
    {
        // URLs are of the form image://webvfx/<name>/<count>
        // where <count> is a unique ID to force refresh and is ignored.
        QImage image(contentContext->getImage(id.section('/', 0, 0)));
        QPixmap pixmap(QPixmap::fromImage(image));

        if (size)
            *size = pixmap.size();

        if (!requestedSize.isEmpty())
            return pixmap.scaled(requestedSize, Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);

        return pixmap;
    }

private:
    ContentContext* contentContext;
 };

////////////////////

QmlContent::QmlContent(const QSize& size, Parameters* parameters)
    : QDeclarativeView(0)
    , pageLoadFinished(LoadNotFinished)
    , contextLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters))
    , syncLoop(0)
{
    if (!s_QmlContentRegistered) {
        s_QmlContentRegistered = true;
        qmlRegisterType<GraphicsCaptureEffect>("org.webvfx.WebVfx", 1, 0, "Capture");
    }
    // Add root of our qrc:/ resource path so embedded QML components are available.
    engine()->addImportPath(":/");

    // Turn off scrollbars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setInteractive(false);
    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    setResizeAnchor(QDeclarativeView::AnchorViewCenter);
    resize(size);

    QGLWidget* glWidget = new QGLWidget();
    setViewport(glWidget);

    renderer.init(glWidget, size);
    renderer.setRenderType(Renderer::RenderGLAntialias);

    // OpenGL needs this
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // Expose context to the QML
    rootContext()->setContextProperty("webvfx", contentContext);

    // Register image provider for image://webvfx/<name>/<counter>
    engine()->addImageProvider(QLatin1String("webvfx"), new PixmapProvider(contentContext));

    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(qmlViewStatusChanged(QDeclarativeView::Status)));
    connect(engine(), SIGNAL(warnings(QList<QDeclarativeError>)), SLOT(logWarnings(QList<QDeclarativeError>)));
    connect(contentContext, SIGNAL(readyRender(bool)), SLOT(contentContextLoadFinished(bool)));
}

void QmlContent::qmlViewStatusChanged(QDeclarativeView::Status status)
{
    if (status != QDeclarativeView::Ready && status != QDeclarativeView::Error)
        return;

    if (pageLoadFinished == LoadNotFinished)
        pageLoadFinished = (status == QDeclarativeView::Ready) ? LoadSucceeded : LoadFailed;
    if (syncLoop && (pageLoadFinished == LoadFailed ||
                     contextLoadFinished != LoadNotFinished))
        syncLoop->exit(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
}

void QmlContent::contentContextLoadFinished(bool result)
{
    if (contextLoadFinished == LoadNotFinished)
        contextLoadFinished = result ? LoadSucceeded : LoadFailed;
    if (syncLoop && (contextLoadFinished == LoadFailed ||
                     pageLoadFinished != LoadNotFinished))
        syncLoop->exit(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
}

void QmlContent::logWarnings(const QList<QDeclarativeError>& warnings)
{
    foreach (const QDeclarativeError& warning, warnings) {
        log(warning.toString());
    }
}

bool QmlContent::loadContent(const QUrl& url)
{
    if (syncLoop) {
        log("QmlContent::loadContent recursive call detected");
        return false;
    }

    pageLoadFinished = LoadNotFinished;
    contextLoadFinished = LoadNotFinished;

    QSize originalSize(size());

    setSource(url);

    // XXX QDeclarativeView::SizeRootObjectToView is broken, so resize after loading
    // http://bugreports.qt.nokia.com/browse/QTBUG-15863
    setContentSize(originalSize);

    bool result = false;
    if (pageLoadFinished != LoadFailed
        && (pageLoadFinished == LoadNotFinished
            || contextLoadFinished == LoadNotFinished)) {
        // Run a nested event loop which will be exited when both
        // qmlViewStatusChanged and contentContextLoadFinished signal,
        // returning the result code here.
        // http://wiki.forum.nokia.com/index.php/How_to_wait_synchronously_for_a_Signal_in_Qt
        // http://qt.gitorious.org/qt/qt/blobs/4.7/src/gui/dialogs/qdialog.cpp#line549
        QEventLoop loop;
        syncLoop = &loop;
        result = loop.exec();
        syncLoop = 0;
    }
    else
        result = pageLoadFinished == LoadSucceeded && contextLoadFinished == LoadSucceeded;

    logWarnings(errors());

    return result;
}

void QmlContent::setContentSize(const QSize& size) {
    QSize oldSize(this->size());
    if (oldSize != size) {
        resize(size);
        renderer.resize(size);
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
    return renderer.render(this, renderImage);
    //XXX also check errors() after each render()
}

void QmlContent::paintContent(QPainter* painter)
{
    render(painter);
}

}
