#include <QApplication>
#include <QFileInfo>
#include <QSize>
#include <QStringBuilder>
#include <QThread>
#include <QUrl>
#include "webvfx/content.h"
#include "webvfx/effects_impl.h"
#include "webvfx/image.h"
#include "webvfx/parameters.h"
#include "webvfx/qml_content.h"
#include "webvfx/web_content.h"
#include "webvfx/webvfx.h"


namespace WebVfx
{

EffectsImpl::EffectsImpl()
    : QObject(0)
    , content(0)
    , loadResult(false)
    , renderResult(false)
{
}

EffectsImpl::~EffectsImpl()
{
    delete content;
}

bool EffectsImpl::initialize(const QString& fileName, int width, int height, Parameters* parameters)
{
    QUrl url(QUrl::fromLocalFile(QFileInfo(fileName).absoluteFilePath()));

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

void EffectsImpl::setImage(const QString& name, Image* image)
{
    // This may create a QImage and modify QHash - both of those classes
    // are reentrant, so should be safe to do on calling thread as long
    // as access to this EffectsImpl is synchronized.
    content->setImage(name, image);
}

bool EffectsImpl::render(double time, Image* renderImage)
{
    if (onUIThread()) {
        renderInvokable(time, renderImage);
    }
    else {
        QMetaObject::invokeMethod(this, "renderInvokable",
                                  Qt::BlockingQueuedConnection,
                                  Q_ARG(double, time),
                                  Q_ARG(Image*, renderImage));
    }
    return renderResult;
}

void EffectsImpl::initializeInvokable(const QUrl& url, const QSize& size, Parameters* parameters)
{
    QString path(url.path());
    // We can't parent QmlContent since we aren't a QWidget.
    // So don't parent either content, and destroy them explicitly.
    if (path.endsWith(".html", Qt::CaseInsensitive))
        content = new WebContent(size, parameters);
    else if (path.endsWith(".qml", Qt::CaseInsensitive))
        content = new QmlContent(size, parameters);
    else {
        log(QLatin1Literal("WebVfx Filename must end with '.html' or '.qml': ") % path);
        loadResult = false;
        return;
    }

    // Qt 4.8.0 allows BlockingQueuedConnection to return a value http://bugreports.qt.nokia.com/browse/QTBUG-10440
    loadResult = content->loadContent(url);
}

void EffectsImpl::renderInvokable(double time, Image* renderImage)
{
    content->setContentSize(QSize(renderImage->width(), renderImage->height()));
    renderResult = content->renderContent(time, renderImage);
}

}
