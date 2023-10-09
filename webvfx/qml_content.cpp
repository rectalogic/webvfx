#include "qml_content.h"
#include "render_control.h" // for RenderControl
#include <QDebug> // for QDebug
#include <QImage> // for QImage
#include <QList> // for QList<>::const_iterator, QList
#include <QQmlContext> // for QQmlContext
#include <QQmlEngine> // for QQmlEngine
#include <QQmlError> // for QQmlError
#include <QQuickView> // for QQuickView, QQuickView::Error, QQuickView::Ready, QQuickView::ResizeMode, QQuickView::SizeViewToRootObject, QQuickView::Status
#include <QSize> // for operator!=, QSize
#include <QUrl> // for QUrl
#include <QtCore> // for QForeachContainer, qMakeForeachContainer
#include <QtGlobal> // for qDebug

namespace WebVfx {

QmlContent::QmlContent(RenderControl* renderControl, Parameters* parameters)
    : QQuickView(QUrl(), renderControl)
    , contentContext(new ContentContext(this, parameters))
    , renderControl(renderControl)
    , renderExpected(false)
{
    setResizeMode(ResizeMode::SizeRootObjectToView);

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
    if (!renderControl->install(this, size())) {
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