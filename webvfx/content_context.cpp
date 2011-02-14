#include <QImage>
#include <QPixmap>
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
    emit renderRequested(time);
}

Image ContentContext::getImage(const QString& name, const QSize& size)
{
    QImage* image = imageMap.value(name);
    if (!image || image->size() != size) {
        delete image;
        image = new QImage(size, QImage::Format_RGB888);
        imageMap.insert(name, image);
    }
    return Image(image->bits(), image->width(), image->height(), image->byteCount());
}

double ContentContext::getNumberParameter(const QString& name)
{
    if (parameters)
        return parameters->getNumberParameter(name);
    else
        return 0;
}

const QString ContentContext::getStringParameter(const QString& name)
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

// QtWebkit Bridge converts QImages to QPixmaps anyway,
// so better to do the conversion here since handing over a QImage
// would cause it's shared data to be duplicated the next time we wrote to it's bits.
// http://doc.qt.nokia.com/latest/qimage.html#bits
const QPixmap ContentContext::getImage(const QString& name)
{
    QImage* image = imageMap.value(name);
    if (image)
        return QPixmap::fromImage(*image);
    else
        return QPixmap();
}

}
