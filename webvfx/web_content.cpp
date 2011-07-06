#include <QEventLoop>
#include <QGLWidget>
#include <QGraphicsScene>
#include <QGraphicsWebView>
#include <QMap>
#include <QPainter>
#include <QSize>
#include <QStringBuilder>
#include <QVariant>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>
#include "webvfx/image.h"
#include "webvfx/renderer.h"
#include "webvfx/web_content.h"
#include "webvfx/webvfx.h"

namespace WebVfx
{

WebPage::WebPage(QObject* parent) : QWebPage(parent) {
    mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    settings()->setAttribute(QWebSettings::SiteSpecificQuirksEnabled, false);
    settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
    settings()->setAttribute(QWebSettings::WebGLEnabled, true);
#endif
}

bool WebPage::shouldInterruptJavaScript() {
    return false;
}

void WebPage::javaScriptAlert(QWebFrame *, const QString &msg) {
    log(QLatin1Literal("JavaScript alert: ") % msg);
}

void WebPage::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID) {
    QString msg(message);
    if (!sourceID.isEmpty())
        msg % QLatin1Literal(" (") % sourceID.section('/', -1) % QLatin1Literal(":") % QString::number(lineNumber) % QLatin1Literal(")");
    log(msg);
}

bool WebPage::acceptNavigationRequest(QWebFrame*, const QNetworkRequest&, NavigationType) {
    //XXX we want to prevent JS from navigating the page - does this prevent our initial load?
    //return false;
    return true;
}

////////////////////

WebContent::WebContent(const QSize& size, Parameters* parameters)
    : QGraphicsView((QWidget*)0)
    , webView(new QGraphicsWebView)
    , webPage(new WebPage(this))
    , pageLoadFinished(LoadNotFinished)
    , contextLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters))
    , syncLoop(0)
    , renderer(0)
{
    // Turn off scrollbars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(false);
    setFrameStyle(0);

    webView->setPage(webPage);
    setScene(new QGraphicsScene(this));
    // Scene owns webView
    scene()->addItem(webView);

    // Must use a QGraphicsWebView with a parent QAbstractScrollArea
    // with a QGLWidget viewport, then WebGL will find this QGLWidget
    // and share it for rendering.
    // See WebKit:
    //   GraphicsContext3DQt.cpp GraphicsContext3DInternal::getViewportGLWidget
    // The QGLWidget also makes WebKit use the GL TextureMapper.
    // See WebKit:
    //   PageClientQt.cpp PageClientQGraphicsWidget::setRootGraphicsLayer
    QGLWidget* glWidget = new QGLWidget;
    setViewport(glWidget);
    // OpenGL needs this
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    renderer = new GLRenderer(glWidget);

    connect(webPage, SIGNAL(loadFinished(bool)),
            SLOT(webPageLoadFinished(bool)));
    connect(contentContext, SIGNAL(readyRender(bool)),
            SLOT(contentContextLoadFinished(bool)));
    connect(webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(injectContentContext()));

    setContentSize(size);
}

WebContent::~WebContent()
{
    delete renderer;
}

void WebContent::injectContentContext()
{
    webPage->mainFrame()->addToJavaScriptWindowObject("webvfx", contentContext);
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

    webPage->mainFrame()->load(url);

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
void WebContent::setContentSize(const QSize& size) {
    if (size != this->size()) {
        webView->resize(size);
        resize(size);
        viewport()->resize(size);
    }
}

bool WebContent::renderContent(double time, Image* renderImage)
{
    // Allow the page to render for this time
    contentContext->render(time);
    return renderer->render(this, renderImage);
}

void WebContent::paintContent(QPainter* painter)
{
    webPage->mainFrame()->render(painter);
}

QWidget* WebContent::createView(QWidget* parent)
{
    setParent(parent);
    return this;
}

QWebSettings* WebContent::settings()
{
    return webPage->settings();
}

}
