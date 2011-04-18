#include <QEventLoop>
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QSize>
#include <QStringBuilder>
#include <QVariant>
#include <QWebFrame>
#include "webvfx/image.h"
#include "webvfx/web_content.h"
#include "webvfx/webvfx.h"

namespace WebVfx
{

WebContent::WebContent(QObject* parent, const QSize& size, Parameters* parameters)
    : QWebPage(parent)
    , pageLoadFinished(LoadNotFinished)
    , contextLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters))
    , syncLoop(0)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(webPageLoadFinished(bool)));
    connect(contentContext, SIGNAL(readyRender(bool)), SLOT(contentContextLoadFinished(bool)));
    connect(mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(injectContentContext()));

    setContentSize(size);

    settings()->setAttribute(QWebSettings::SiteSpecificQuirksEnabled, false);
    settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
    settings()->setAttribute(QWebSettings::WebGLEnabled, true);
#endif

    // Turn off scrollbars
    mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
}

WebContent::~WebContent()
{
}

void WebContent::javaScriptAlert(QWebFrame *, const QString &msg)
{
    log(QLatin1Literal("JavaScript alert: ") % msg);
}

void WebContent::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID)
{
    QString msg(message);
    if (!sourceID.isEmpty())
        msg % QLatin1Literal(" (") % sourceID.section('/', -1) % QLatin1Literal(":") % QString::number(lineNumber) % QLatin1Literal(")");
    log(msg);
}

bool WebContent::acceptNavigationRequest(QWebFrame*, const QNetworkRequest&, NavigationType)
{
    //XXX we want to prevent JS from navigating the page - does this prevent our initial load?
    //return false;
    return true;
}

void WebContent::injectContentContext()
{
    mainFrame()->addToJavaScriptWindowObject("webvfx", contentContext);
}

bool WebContent::shouldInterruptJavaScript()
{
    return false;
}

void WebContent::webPageLoadFinished(bool result)
{
    if (pageLoadFinished == LoadNotFinished)
        pageLoadFinished = result ? LoadSucceeded : LoadFailed;
    if (syncLoop && (pageLoadFinished == LoadFailed ||
                     contextLoadFinished != LoadNotFinished))
        syncLoop->exit(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
}

void WebContent::contentContextLoadFinished(bool result)
{
    if (contextLoadFinished == LoadNotFinished)
        contextLoadFinished = result ? LoadSucceeded : LoadFailed;
    if (syncLoop && (contextLoadFinished == LoadFailed ||
                     pageLoadFinished != LoadNotFinished))
        syncLoop->exit(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
}

bool WebContent::loadContent(const QUrl& url)
{
    if (syncLoop) {
        log("WebContent::loadContent recursive call detected");
        return false;
    }

    pageLoadFinished = LoadNotFinished;
    contextLoadFinished = LoadNotFinished;

    mainFrame()->load(url);

    if (pageLoadFinished != LoadFailed
        && (pageLoadFinished == LoadNotFinished
            || contextLoadFinished == LoadNotFinished)) {
        // Run a nested event loop which will be exited when both
        // webPageLoadFinished and contentContextLoadFinished signal,
        // returning the result code here.
        // http://wiki.forum.nokia.com/index.php/How_to_wait_synchronously_for_a_Signal_in_Qt
        // http://qt.gitorious.org/qt/qt/blobs/4.7/src/gui/dialogs/qdialog.cpp#line549
        QEventLoop loop;
        syncLoop = &loop;
        bool result = loop.exec();
        syncLoop = 0;
        return result;
    }
    else
        return pageLoadFinished == LoadSucceeded && contextLoadFinished == LoadSucceeded;
}

bool WebContent::renderContent(double time, Image* renderImage)
{
    // Allow the page to render for this time
    contentContext->render(time);

    if (!renderImage)
        return false;

    // QImage referencing our Image bits
    QImage image((uchar*)renderImage->pixels(), renderImage->width(),
                 renderImage->height(), renderImage->bytesPerLine(),
                 QImage::Format_RGB888);

    // Render frame into image
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    mainFrame()->render(&painter);
    painter.end();

    return true;
}

}
