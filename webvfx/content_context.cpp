#include <QImage>
#include <QVariant>
#include <QtAlgorithms>
#include "webvfx/content_context.h"
#include "webvfx/parameters.h"

namespace WebVfx
{

ContentContext::ContentContext(QObject* parent, Parameters* parameters, QSize size)
    : QObject(parent)
    , parameters(parameters)
    , renderCount(0)
    , videoSize(size)
{
}

ContentContext::~ContentContext()
{
    delete parameters;
}

void ContentContext::render(double time)
{
    renderCount++;
    emit renderRequested(time);
}

void ContentContext::setImage(const QString& name, QImage image)
{
    imageMap.insert(name, image);
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

void ContentContext::registerImageName(const QString& imageName) {
    imageNameSet.insert(imageName);
}

// QtWebkit Bridge converts QImages to QPixmaps.
// QML Qt3D QDeclarativeEffect::setTextureImage also does,
// and it takes a QImage - so better to return QImage here
// instead of QPixmap to avoid too many conversions.
// One issue is the next time we write to the QImage, it's shared data
// will be duplicated. But since it is being transferred to a QPixmap,
// this shouldn't be an issue (i.e. we should be the only reference).
// http://doc.qt.nokia.com/latest/qimage.html#bits
QImage ContentContext::getImage(const QString& name)
{
    return imageMap.value(name);
}

QUrl ContentContext::getImageUrl(const QString& name)
{
    // Include renderCount in URL - this makes the URL unique
    // so QML will actually reload the image.
    return QUrl::fromEncoded(QString("image://webvfx/%1/%2").arg(name).arg(renderCount).toLatin1(), QUrl::StrictMode);
}

void ContentContext::setVideoSize(QSize size) {
    if (videoSize == size)
        return;
    videoSize = size;
    emit videoSizeChanged();
}

}
