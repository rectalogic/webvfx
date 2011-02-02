#include <QApplication>
#include <QThread>
#include "webfx/web_logger.h"
#include "webfx/web_page.h"
#include "webfx/web_parameters.h"
#include "webfx/web_renderer.h"
#include "webfx/web_script.h"


WebFX::WebRenderer::WebRenderer(QObject* parent)
    : QObject(parent)
    , webPage(0)
    , webScript(0)
{
}

WebFX::WebRenderer::~WebRenderer()
{
    delete webPage;
    delete webScript;
}

bool WebFX::WebRenderer::initialize(const std::string& url, int width, int height, WebFX::WebParameters* parameters)
{
    QUrl qurl(QString::fromStdString(url));

    if (!qurl.isValid()) {
        WebFX::log(std::string("Invalid URL: ") + url);
        return false;
    }

    QSize size(width, height);

    if (onUIThread()) {
        initializeInvokable(qurl, size, parameters);
    }
    else {
        // Move ourself onto GUI thread and create our WebPage there
        this->moveToThread(QApplication::instance()->thread());
        QMetaObject::invokeMethod(this, "initializeInvokable", Qt::BlockingQueuedConnection,
                                  Q_ARG(QUrl, qurl), Q_ARG(QSize, size), Q_ARG(void*, parameters));
    }

    //XXX check actual load status

    return true;
}

void WebFX::WebRenderer::destroy()
{
    deleteLater();
}

bool WebFX::WebRenderer::onUIThread() {
    return QThread::currentThread() == QApplication::instance()->thread();
}

void WebFX::WebRenderer::setSize(int width, int height)
{
    QSize size(width, height);

    if (onUIThread()) {
        setSizeInvokable(size);
    }
    else {
        QMetaObject::invokeMethod(this, "setSizeInvokable", Qt::BlockingQueuedConnection,
                                  Q_ARG(QSize, size));
    }
}

void WebFX::WebRenderer::initializeInvokable(const QUrl& url, const QSize& size, WebFX::WebParameters* parameters)
{
    webPage = new WebFX::WebPage();
    webPage->setViewportSize(size);

    //XXX hookup signals
    //XXX install "webfx" object into page - another Qt class we can own here
    connect(webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(inject()));

    //XXX we should enable webgl for our QtWebKit builds
    webPage->settings()->setAttribute(QWebSettings::SiteSpecificQuirksEnabled, false);
    webPage->settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, false);

    // Turn off scrollbars
    webPage->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    webPage->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    //XXX need to get result back to initialize()
    bool result = webPage->loadSync(url);
    // 4.8.0 allows BlockingQueuedConnection to return a value http://bugreports.qt.nokia.com/browse/QTBUG-10440

}

void WebFX::WebRenderer::setSizeInvokable(const QSize& size)
{
    webPage->setViewportSize(size);
}
