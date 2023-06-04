#include "qml_content.h"
#include "render_control.h"
#include "webvfx.h"
#include <QImage>
#include <QList>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickRenderControl>
#include <QQuickView>
#include <QSize>

namespace WebVfx {

QmlContent::QmlContent(RenderControl* renderControl, Parameters* parameters)
    : QQuickView(QUrl(), renderControl)
    , pageLoadFinished(LoadNotFinished)
    , contentContext(new ContentContext(this, parameters))
    , renderControl(renderControl)
{
    setResizeMode(ResizeMode::SizeViewToRootObject);

    // Expose context to the QML
    rootContext()->setContextProperty("webvfx", contentContext);

    connect(this, SIGNAL(statusChanged(QQuickView::Status)), SLOT(qmlViewStatusChanged(QQuickView::Status)));
    connect(engine(), SIGNAL(warnings(QList<QQmlError>)), SLOT(logWarnings(QList<QQmlError>)));
}

QmlContent::QmlContent(Parameters* parameters)
    : QmlContent(new RenderControl(), parameters)
{
}

QmlContent::~QmlContent()
{
}

void QmlContent::qmlViewStatusChanged(QQuickView::Status status)
{
    if (status != QQuickView::Ready && status != QQuickView::Error)
        return;

    if (pageLoadFinished == LoadNotFinished)
        pageLoadFinished = (status == QQuickView::Ready) ? LoadSucceeded : LoadFailed;

    logWarnings(errors());
    emit contentLoadFinished(pageLoadFinished == LoadSucceeded);
}

void QmlContent::logWarnings(const QList<QQmlError>& warnings)
{
    foreach (const QQmlError& warning, warnings) {
        qDebug() << warning.toString();
    }
}

void QmlContent::loadContent(const QUrl& url)
{
    pageLoadFinished = LoadNotFinished;
    setSource(url);
}

void QmlContent::setContentSize(const QSize& size)
{
    if (contentContext->getVideoSize() != size) {
        contentContext->setVideoSize(size);
    }
}

QImage QmlContent::renderContent(double time)
{
    if (!renderControl->install(this, contentContext->getVideoSize())) {
        return QImage();
    }

    // Allow the content to render for this time
    contentContext->render(time);

    if (status() == QQuickView::Error) {
        logWarnings(errors());
    }

    return renderControl->renderImage();
}

}