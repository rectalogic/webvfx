#include <QApplication>
#include <QSize>
#include <QStringBuilder>
#include <QThread>
#include <QUrl>
#include "webvfx/content.h"
#include "webvfx/effects_impl.h"
#include "webvfx/image.h"
#include "webvfx/logger.h"
#include "webvfx/parameters.h"
#include "webvfx/web_content.h"


namespace WebVfx
{

EffectsImpl::EffectsImpl()
    : QObject(0)
    , content(0)
    , loadResult(false)
{
}

bool EffectsImpl::initialize(const QString& fileName, int width, int height, Parameters* parameters)
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
        // Move ourself onto GUI thread and create our Content there
        this->moveToThread(QApplication::instance()->thread());
        QMetaObject::invokeMethod(this, "initializeInvokable", Qt::BlockingQueuedConnection,
                                  Q_ARG(QUrl, url), Q_ARG(QSize, size), Q_ARG(Parameters*, parameters));
    }

    return loadResult;
}

void EffectsImpl::destroy()
{
    deleteLater();
}

bool EffectsImpl::onUIThread() {
    return QThread::currentThread() == QApplication::instance()->thread();
}

const Effects::ImageTypeMap& EffectsImpl::getImageTypeMap()
{
    return content->getImageTypeMap();
}

Image EffectsImpl::getImage(const QString& name, int width, int height)
{
    // This may create a QImage and modify QHash - both of those classes are reentrant,
    // so should be safe to do on calling thread as long as access to this EffectsImpl is synchronized.
    return content->getImage(name, QSize(width, height));
}

const Image EffectsImpl::render(double time, int width, int height)
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

void EffectsImpl::initializeInvokable(const QUrl& url, const QSize& size, Parameters* parameters)
{
    content = new WebContent(this, size, parameters);
	//XXX figure out which to create based on url
    //content = new QmlContent(this, size, parameters);

    // Qt 4.8.0 allows BlockingQueuedConnection to return a value http://bugreports.qt.nokia.com/browse/QTBUG-10440
    loadResult = content->loadContent(url);
}

void EffectsImpl::renderInvokable(double time, const QSize& size)
{
    content->setContentSize(size);
    renderImage = content->renderContent(time);
}

}
