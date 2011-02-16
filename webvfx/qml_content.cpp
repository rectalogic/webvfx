#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeImageProvider>
#include <QDeclarativeError>
#include <QEventLoop>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QSize>
#include <QString>
#include <QVariant>
#include "webvfx/image.h"
#include "webvfx/logger.h"
#include "webvfx/qml_content.h"


namespace WebVfx
{

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
	QPixmap pixmap(contentContext->getImage(id.section('/', 0, 0)));

	if (size)
	    *size = pixmap.size();

	if (!requestedSize.isEmpty())
	    return pixmap.scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	return pixmap;
    }

private:
    ContentContext* contentContext;
 };

////////////////////

QmlContent::QmlContent(QWidget* parent, QSize size, Parameters* parameters)
    : QDeclarativeView(parent)
    , pageLoadFinished(LoadNotFinished)
    , contextLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters))
    , syncLoop(0)
    , renderImage(0)
{
    // Expose context to the QML
    rootContext()->setContextProperty("webvfx", contentContext);

    // Register image provider for image://webvfx/<name>/<counter>
    engine()->addImageProvider(QLatin1String("webvfx"), new PixmapProvider(contentContext));

    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(qmlViewStatusChanged(QDeclarativeView::Status)));
    connect(engine(), SIGNAL(warnings(QList<QDeclarativeError>)), SLOT(logWarnings(QList<QDeclarativeError>)));
    connect(contentContext, SIGNAL(loadFinished(bool)), SLOT(contentContextLoadFinished(bool)));

    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    setContentSize(size);

    // Turn off scrollbars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

QmlContent::~QmlContent()
{
    delete renderImage;
}

void QmlContent::qmlViewStatusChanged(QDeclarativeView::Status status)
{
    if (status != QDeclarativeView::Ready && status != QDeclarativeView::Error)
        return;

    if (pageLoadFinished == LoadNotFinished)
        pageLoadFinished = (status == QDeclarativeView::Ready) ? LoadSucceeded : LoadFailed;
    if (syncLoop && contextLoadFinished != LoadNotFinished)
        syncLoop->exit(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
}

void QmlContent::contentContextLoadFinished(bool result)
{
    if (contextLoadFinished == LoadNotFinished)
        contextLoadFinished = result ? LoadSucceeded : LoadFailed;
    if (syncLoop && pageLoadFinished != LoadNotFinished)
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
    if (pageLoadFinished == LoadNotFinished || contextLoadFinished == LoadNotFinished) {
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

Image QmlContent::renderContent(double time)
{
    // Allow the content to render for this time
    contentContext->render(time);

    // Create/recreate image with correct size
    QSize size(this->size());
    if (!renderImage)
        renderImage = new QImage(size, QImage::Format_RGB888);
    else if (renderImage->size() != size) {
        delete renderImage;
        renderImage = new QImage(size, QImage::Format_RGB888);
    }

    // Render frame into image
    QPainter painter(renderImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    render(&painter);
    painter.end();

    // Return Image referencing our bits
    return Image(renderImage->bits(), renderImage->width(), renderImage->height(), renderImage->byteCount());
    //XXX also check errors() after each render()
}

}
