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
    , contentContext(new ContentContext(this, parameters))
    , renderControl(renderControl)
    , renderExpected(false)
{
    setResizeMode(ResizeMode::SizeViewToRootObject);

    // Expose context to the QML
    rootContext()->setContextProperty("webvfx", contentContext);

    connect(this, SIGNAL(statusChanged(QQuickView::Status)), SLOT(qmlViewStatusChanged(QQuickView::Status)));
    connect(engine(), SIGNAL(warnings(QList<QQmlError>)), SLOT(logWarnings(QList<QQmlError>)));
    connect(contentContext, SIGNAL(asyncRenderComplete()), SLOT(contextAsyncRenderComplete()));
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

    logWarnings(errors());
    emit contentLoadFinished(status == QQuickView::Ready);
}

void QmlContent::logWarnings(const QList<QQmlError>& warnings)
{
    foreach (const QQmlError& warning, warnings) {
        qDebug() << warning.toString();
    }
}

void QmlContent::loadContent(const QUrl& url)
{
    setSource(url);
}

void QmlContent::setContentSize(const QSize& size)
{
    if (contentContext->getVideoSize() != size) {
        contentContext->setVideoSize(size);
    }
}

void QmlContent::contextAsyncRenderComplete()
{
    if (!renderExpected) {
        qDebug() << "Unexpected emitAsyncRenderComplete()";
        emit renderComplete(QImage());
        return;
    }
    renderExpected = false;
    if (status() == QQuickView::Error) {
        logWarnings(errors());
    }
    emit renderComplete(renderControl->renderImage());
}

void QmlContent::renderContent(double time)
{
    if (!renderControl->install(this, contentContext->getVideoSize())) {
        emit renderComplete(QImage());
    }

    // Allow the content to render for this time
    renderExpected = true;
    contentContext->render(time);

    if (!contentContext->isAsyncRenderRequired()) {
        contextAsyncRenderComplete();
    }
}

}