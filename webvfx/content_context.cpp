#include "webvfx/content_context.h"
#include "webvfx/parameters.h"
//#include <QVariant>
//#include <QtAlgorithms>

namespace WebVfx {

ContentContext::ContentContext(QObject* parent, Parameters* parameters, QSize size)
    : QObject(parent)
    , parameters(parameters)
    , videoSize(size)
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
}

void ContentContext::addVideoSink(QVideoSink* videoSink)
{
    videoSinks.append(videoSink);
}

}
