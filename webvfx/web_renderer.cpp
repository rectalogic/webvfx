#include <QApplication>
#include <QThread>
#include <QWebFrame>
#include <QWebSettings>
#include "webvfx/image.h"
#include "webvfx/logger.h"
#include "webvfx/web_page.h"
#include "webvfx/parameters.h"
#include "webvfx/web_renderer.h"


namespace WebVFX
{

WebRenderer::WebRenderer()
    : QObject(0)
    , webPage(0)
    , loadResult(false)
{
}

bool WebRenderer::initialize(const std::string& url, int width, int height, Parameters* parameters)
{
    QUrl qurl(QString::fromStdString(url));

    if (!qurl.isValid()) {
        log(std::string("Invalid URL: ") + url);
        return false;
    }

    QSize size(width, height);

    loadResult = false;

    if (onUIThread()) {
        initializeInvokable(qurl, size, parameters);
    }
    else {
        // Move ourself onto GUI thread and create our WebPage there
        this->moveToThread(QApplication::instance()->thread());
        QMetaObject::invokeMethod(this, "initializeInvokable", Qt::BlockingQueuedConnection,
                                  Q_ARG(QUrl, qurl), Q_ARG(QSize, size), Q_ARG(Parameters*, parameters));
    }

    return loadResult;
}

void WebRenderer::destroy()
{
    deleteLater();
}

bool WebRenderer::onUIThread() {
    return QThread::currentThread() == QApplication::instance()->thread();
}

const WebEffects::ImageTypeMap& WebRenderer::getImageTypeMap()
{
    return webPage->getImageTypeMap();
}

Image WebRenderer::getImage(const std::string& name, int width, int height)
{
    // This may create a QImage and modify QHash - both of those classes are reentrant,
    // so should be safe to do on calling thread as long as access to this WebRenderer is synchronized.
    return webPage->getImage(QString::fromStdString(name), QSize(width, height));
}

const Image WebRenderer::render(double time, int width, int height)
{
    QSize size(width, height);

    if (onUIThread()) {
        renderInvokable(time, size);
    }
    else {
        QMetaObject::invokeMethod(this, "renderInvokable", Qt::BlockingQueuedConnection,
                                  Q_ARG(double, time), Q_ARG(QSize, size));
    }
    return renderImage;
}

void WebRenderer::initializeInvokable(const QUrl& url, const QSize& size, Parameters* parameters)
{
    webPage = new WebPage(this, size, parameters);

    // Qt 4.8.0 allows BlockingQueuedConnection to return a value http://bugreports.qt.nokia.com/browse/QTBUG-10440
    loadResult = webPage->loadSync(url);
}

void WebRenderer::renderInvokable(double time, const QSize& size)
{
    webPage->setViewportSize(size);
    renderImage = webPage->render(time);
}

}
