#include <sstream>
#include <QEventLoop>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QVariant>
#include <QWebFrame>
#include "webvfx/logger.h"
#include "webvfx/web_page.h"

namespace WebVfx
{

WebPage::WebPage(QObject* parent, QSize size, Parameters* parameters)
    : QWebPage(parent)
    , pageLoadFinished(LoadNotFinished)
    , contextLoadFinished(LoadNotFinished)
    , effectsContext(new EffectsContext(this, parameters))
    , syncLoop(0)
    , renderImage(0)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(webPageLoadFinished(bool)));
    connect(effectsContext, SIGNAL(loadFinished(bool,QVariantMap)), SLOT(effectsContextLoadFinished(bool,QVariantMap)));
    connect(mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(injectEffectsContext()));

    setViewportSize(size);

    settings()->setAttribute(QWebSettings::SiteSpecificQuirksEnabled, false);
    settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, false);
#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
    settings()->setAttribute(QWebSettings::WebGLEnabled, true);
#endif

    // Turn off scrollbars
    mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
}

WebPage::~WebPage()
{
    delete renderImage;
}

void WebPage::javaScriptAlert(QWebFrame *originatingFrame, const QString &msg)
{
    Q_UNUSED(originatingFrame);
    log(std::string("JavaScript alert: ") + msg.toStdString());
}

void WebPage::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID)
{
    std::ostringstream oss;
    oss << message.toStdString();
    if (!sourceID.isEmpty())
        oss << " (" << sourceID.section('/', -1).toStdString() << ":" << lineNumber << ")";
    log(oss.str());
}

bool WebPage::acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest& request, NavigationType type)
{
    Q_UNUSED(frame);
    Q_UNUSED(request);
    Q_UNUSED(type);
    //XXX we want to prevent JS from navigating the page - does this prevent our initial load?
    //return false;
    return true;
}

void WebPage::injectEffectsContext()
{
    mainFrame()->addToJavaScriptWindowObject("webvfx", effectsContext);
}

bool WebPage::shouldInterruptJavaScript()
{
    return false;
}

void WebPage::webPageLoadFinished(bool result)
{
    if (pageLoadFinished == LoadNotFinished)
        pageLoadFinished = result ? LoadSucceeded : LoadFailed;
    if (syncLoop && contextLoadFinished != LoadNotFinished)
        syncLoop->exit(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
}

void WebPage::effectsContextLoadFinished(bool result, const QVariantMap& typeMap)
{
    if (contextLoadFinished == LoadNotFinished) {
        contextLoadFinished = result ? LoadSucceeded : LoadFailed;
        // Convert QVariantMap to std::map
        QMapIterator<QString, QVariant> iter(typeMap);
        while (iter.hasNext()) {
            iter.next();
            //XXX validate the type enums
            imageTypeMap[iter.key().toStdString()] = static_cast<Effects::ImageType>(iter.value().toInt());
        }
    }
    if (syncLoop && pageLoadFinished != LoadNotFinished)
        syncLoop->exit(contextLoadFinished == LoadSucceeded && pageLoadFinished == LoadSucceeded);
}

bool WebPage::loadSync(const QUrl& url)
{
    if (syncLoop) {
        log("WebPage::loadSync recursive call detected");
        return false;
    }

    pageLoadFinished = LoadNotFinished;
    contextLoadFinished = LoadNotFinished;

    mainFrame()->load(url);

    // Run a nested event loop which will be exited when both
    // webPageLoadFinished and effectsContextLoadFinished signal,
    // returning the result code here.
    // http://wiki.forum.nokia.com/index.php/How_to_wait_synchronously_for_a_Signal_in_Qt
    // http://qt.gitorious.org/qt/qt/blobs/4.7/src/gui/dialogs/qdialog.cpp#line549
    QEventLoop loop;
    syncLoop = &loop;
    bool result = loop.exec();
    syncLoop = 0;
    return result;
}

Image WebPage::render(double time)
{
    // Allow the page to render for this time
    effectsContext->render(time);

    // Create/recreate image with correct size
    QSize size = viewportSize();
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
    mainFrame()->render(&painter);
    painter.end();

    // Return Image referencing our bits
    return Image(renderImage->bits(), renderImage->width(), renderImage->height(), renderImage->byteCount());
}

}
