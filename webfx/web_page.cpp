#include <sstream>
#include <QEventLoop>
#include <QImage>
#include <QPainter>
#include <QWebFrame>
#include "webfx/web_logger.h"
#include "webfx/web_page.h"
#include "webfx/web_renderer.h"
#include "webfx/web_script.h"

WebFX::WebPage::WebPage(WebFX::WebRenderer* parent, WebFX::WebParameters* parameters)
    : QWebPage(parent)
    , pageLoadFinished(WebFX::WebPage::LoadNotFinished)
    , scriptLoadFinished(WebFX::WebPage::LoadNotFinished)
    , webScript(new WebFX::WebScript(this, parameters))
    , syncLoop(0)
    , renderImage(0)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(webPageLoadFinished(bool)));
    connect(webScript, SIGNAL(loadFinished(bool)), SLOT(webScriptLoadFinished(bool)));
    connect(mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(injectWebScript()));
}

WebFX::WebPage::~WebPage()
{
    delete renderImage;
}

void WebFX::WebPage::javaScriptAlert(QWebFrame *originatingFrame, const QString &msg)
{
    Q_UNUSED(originatingFrame);
    WebFX::log(std::string("JavaScript alert: ") + msg.toStdString());
}

void WebFX::WebPage::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID)
{
    std::ostringstream oss;
    if (!sourceID.isEmpty())
        oss << sourceID.toStdString() << ":" << lineNumber << " ";
    oss << message.toStdString();
    WebFX::log(oss.str());
}

bool WebFX::WebPage::acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest& request, NavigationType type)
{
    Q_UNUSED(frame);
    Q_UNUSED(request);
    Q_UNUSED(type);
    //XXX we want to prevent JS from navigating the page - does this prevent our initial load?
    //return false;
    return true;
}

void WebFX::WebPage::injectWebScript()
{
    mainFrame()->addToJavaScriptWindowObject("webfx", webScript);
}

bool WebFX::WebPage::shouldInterruptJavaScript()
{
    return false;
}

void WebFX::WebPage::webPageLoadFinished(bool result)
{
    pageLoadFinished = result ? WebFX::WebPage::LoadSucceeded : WebFX::WebPage::LoadFailed;
    if (syncLoop && scriptLoadFinished != WebFX::WebPage::LoadNotFinished)
        syncLoop->exit(scriptLoadFinished == WebFX::WebPage::LoadSucceeded && pageLoadFinished == WebFX::WebPage::LoadSucceeded);
}

void WebFX::WebPage::webScriptLoadFinished(bool result)
{
    scriptLoadFinished = result ? WebFX::WebPage::LoadSucceeded : WebFX::WebPage::LoadFailed;
    if (syncLoop && pageLoadFinished != WebFX::WebPage::LoadNotFinished)
        syncLoop->exit(scriptLoadFinished == WebFX::WebPage::LoadSucceeded && pageLoadFinished == WebFX::WebPage::LoadSucceeded);
}

bool WebFX::WebPage::loadSync(const QUrl& url)
{
    if (syncLoop) {
        WebFX::log("loadSync recursive call detected");
        return false;
    }

    pageLoadFinished = WebFX::WebPage::LoadNotFinished;
    scriptLoadFinished = WebFX::WebPage::LoadNotFinished;

    mainFrame()->load(url);

    // Run a nested event loop which will be exited when both
    // webPageLoadFinished and webScriptLoadFinished signal,
    // returning the result code here.
    // http://wiki.forum.nokia.com/index.php/How_to_wait_synchronously_for_a_Signal_in_Qt
    // http://qt.gitorious.org/qt/qt/blobs/4.7/src/gui/dialogs/qdialog.cpp#line549
    QEventLoop loop;
    syncLoop = &loop;
    bool result = loop.exec();
    syncLoop = 0;
    return result;
}

WebFX::WebImage WebFX::WebPage::render(double time)
{
    // Allow the page to render for this time
    webScript->render(time);

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

    // Return WebImage referencing our bits
    return WebFX::WebImage(renderImage->bits(), renderImage->width(), renderImage->height(), renderImage->byteCount());
}
