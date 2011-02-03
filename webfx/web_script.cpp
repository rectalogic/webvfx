#include <QImage>
#include <QPixmap>
#include <QtAlgorithms>
#include "webfx/web_image.h"
#include "webfx/web_parameters.h"
#include "webfx/web_page.h"
#include "webfx/web_script.h"

WebFX::WebScript::WebScript(WebFX::WebPage* parent, WebFX::WebParameters* parameters)
    : QObject(parent)
    , parameters(parameters)
{
}

WebFX::WebScript::~WebScript()
{
    delete parameters;
    // Delete all images in map
    qDeleteAll(imageMap);
}

void WebFX::WebScript::render(double time)
{
    emit renderRequested(time);
}

WebFX::WebImage WebFX::WebScript::getWebImage(const QString& name, const QSize& size)
{
    QImage* image = imageMap.value(name);
    if (!image || image->size() != size) {
        delete image;
        image = new QImage(size, QImage::Format_RGB888);
        imageMap.insert(name, image);
    }
    return WebFX::WebImage(image->bits(), image->width(), image->height(), image->byteCount());
}

double WebFX::WebScript::getNumberParameter(const QString& name)
{
    if (parameters)
        return parameters->getNumberParameter(name.toStdString());
    else
        return 0;
}

const QString WebFX::WebScript::getStringParameter(const QString& name)
{
    if (parameters)
        return QString::fromStdString(parameters->getStringParameter(name.toStdString()));
    else
        return QString();
}

// QtWebkit Bridge converts QImages to QPixmaps anyway,
// so better to do the conversion here since handing over a QImage
// would cause it's shared data to be duplicated the next time we wrote to it's bits.
// http://doc.qt.nokia.com/latest/qimage.html#bits
const QPixmap WebFX::WebScript::getImage(const QString& name)
{
    QImage* image = imageMap.value(name);
    if (image)
        return QPixmap::fromImage(*image);
    else
        return QPixmap();
}
