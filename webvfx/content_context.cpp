#include <QImage>
#include <QVariant>
#include <QtAlgorithms>
#include "webvfx/content_context.h"
#include "webvfx/image.h"
#include "webvfx/parameters.h"

namespace WebVfx
{

ContentContext::ContentContext(QObject* parent, Parameters* parameters)
    : QObject(parent)
    , parameters(parameters)
    , renderCount(0)
{
}

ContentContext::~ContentContext()
{
    delete parameters;
    // Delete all images in map
    qDeleteAll(imageMap);
}

void ContentContext::render(double time)
{
    renderCount++;
    emit renderRequested(time);

    // Delete all QImage wrappers - data is not valid after render()
    qDeleteAll(imageMap);
    imageMap.clear();
}

void ContentContext::setImage(const QString& name, Image* image)
{
    QImage* qimage = imageMap.value(name);
    // Delete if we already had one
    delete qimage;
    // Create a QImage wrapper for the image data
    qimage = new QImage((uchar*)image->pixels(),
                        image->width(), image->height(),
                        image->bytesPerLine(), QImage::Format_RGB888);
    imageMap.insert(name, qimage);
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

void ContentContext::setImageTypeMap(const QVariantMap& variantMap)
{
    // Convert QVariantMap to ImageTypeMap
    QMapIterator<QString, QVariant> it(variantMap);
    while (it.hasNext()) {
        it.next();
        //XXX validate the type enums
        imageTypeMap[it.key()] = static_cast<Effects::ImageType>(it.value().toInt());
    }
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
    return *imageMap.value(name);
}

QUrl ContentContext::getImageUrl(const QString& name)
{
    // Include renderCount in URL - this makes the URL unique
    // so QML will actually reload the image.
    return QUrl::fromEncoded(QString("image://webvfx/%1/%2").arg(name).arg(renderCount).toAscii(), QUrl::StrictMode);
}

}
