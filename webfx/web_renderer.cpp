#include <QApplication>
#include <QThread>
#include "chromix/web_renderer.h"
#include "chromix/web_page.h"
#include "chromix/web_script.h"
#include "chromix/parameters.h"
#include "chromix/logger.h"


Chromix::WebRenderer::WebRenderer(QObject* parent)
    : QObject(parent)
    , webPage(0)
    , webScript(0)
{
}

Chromix::WebRenderer::~WebRenderer()
{
    delete webPage;
    delete webScript;
}

bool Chromix::WebRenderer::initialize(const std::string& url, int width, int height, Chromix::Parameters* parameters)
{
    QUrl qurl(QString::fromStdString(url));

    if (!qurl.isValid()) {
        Chromix::log(std::string("Invalid URL: ") + url);
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

void Chromix::WebRenderer::destroy()
{
    deleteLater();
}

bool Chromix::WebRenderer::onUIThread() {
    return QThread::currentThread() == QApplication::instance()->thread();
}

void Chromix::WebRenderer::setSize(int width, int height)
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

void Chromix::WebRenderer::initializeInvokable(const QUrl& url, const QSize& size, Chromix::Parameters* parameters)
{
    webPage = new Chromix::WebPage();
    webPage->setViewportSize(size);

    //XXX hookup signals
    //XXX install "chromix" object into page - another Qt class we can own here
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

void Chromix::WebRenderer::setSizeInvokable(const QSize& size)
{
    webPage->setViewportSize(size);
}
