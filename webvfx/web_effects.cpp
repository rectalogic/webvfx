#include <QApplication>
#include <QSize>
#include <QStringBuilder>
#include <QThread>
#include <QUrl>
#include <QWebFrame>
#include <QWebSettings>
#include "webvfx/image.h"
#include "webvfx/logger.h"
#include "webvfx/parameters.h"
#include "webvfx/web_effects.h"
#include "webvfx/web_page.h"


namespace WebVfx
{

WebEffects::WebEffects()
    : QObject(0)
    , webPage(0)
    , loadResult(false)
{
}

bool WebEffects::initialize(const QString& fileName, int width, int height, Parameters* parameters)
{
    QUrl url(QUrl::fromLocalFile(fileName));

    if (!url.isValid()) {
        log(QLatin1Literal("Invalid URL: ") % fileName);
        return false;
    }

    QSize size(width, height);

    loadResult = false;

    if (onUIThread()) {
        initializeInvokable(url, size, parameters);
    }
    else {
        // Move ourself onto GUI thread and create our WebPage there
        this->moveToThread(QApplication::instance()->thread());
        QMetaObject::invokeMethod(this, "initializeInvokable", Qt::BlockingQueuedConnection,
                                  Q_ARG(QUrl, url), Q_ARG(QSize, size), Q_ARG(Parameters*, parameters));
    }

    return loadResult;
}

void WebEffects::destroy()
{
    deleteLater();
}

bool WebEffects::onUIThread() {
    return QThread::currentThread() == QApplication::instance()->thread();
}

const Effects::ImageTypeMap& WebEffects::getImageTypeMap()
{
    return webPage->getImageTypeMap();
}

Image WebEffects::getImage(const QString& name, int width, int height)
{
    // This may create a QImage and modify QHash - both of those classes are reentrant,
    // so should be safe to do on calling thread as long as access to this WebEffects is synchronized.
    return webPage->getImage(name, QSize(width, height));
}

const Image WebEffects::render(double time, int width, int height)
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

void WebEffects::initializeInvokable(const QUrl& url, const QSize& size, Parameters* parameters)
{
    webPage = new WebPage(this, size, parameters);

    // Qt 4.8.0 allows BlockingQueuedConnection to return a value http://bugreports.qt.nokia.com/browse/QTBUG-10440
    loadResult = webPage->loadSync(url);
}

void WebEffects::renderInvokable(double time, const QSize& size)
{
    webPage->setViewportSize(size);
    renderImage = webPage->render(time);
}

}
