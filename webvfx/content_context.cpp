#include <QQmlEngine>

#include "content_context.h"
#include "parameters.h"

namespace WebVfx {

ContentContext::ContentContext(QObject* parent, Parameters* parameters)
    : QObject(parent)
    , parameters(parameters)
    , asyncRenderRequired(false)
{
}

ContentContext::~ContentContext()
{
    delete parameters;
}

void ContentContext::render(double time)
{
    emit renderRequested(time);
}

double ContentContext::getNumberParameter(const QString& name)
{
    if (parameters)
        return parameters->getNumberParameter(name);
    else
        return 0;
}

QString ContentContext::getStringParameter(const QString& name)
{
    if (parameters)
        return parameters->getStringParameter(name);
    else
        return QString();
}

void ContentContext::setVideoSize(QSize size)
{
    if (videoSize == size)
        return;
    videoSize = size;
    emit videoSizeChanged(size);
}

qsizetype ContentContext::addVideoSource()
{
    videoSinks.resize(videoSinks.size() + 1);
    return videoSinks.size() - 1;
}

void ContentContext::appendVideoSink(qsizetype source, QVideoSink* videoSink)
{
    if (source < 0 || source >= videoSinks.size()) {
        qmlEngine(this)->throwError(QStringLiteral("Invalid video source %1").arg(source));
        return;
    }
    videoSinks[source].append(videoSink);
}

void ContentContext::emitAsyncRenderComplete()
{
    emit asyncRenderComplete();
}

}
