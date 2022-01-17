// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QBrush>
#include <QColorDialog>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include "image_color.h"

ImageColor::ImageColor(QWidget* parent)
    : QPushButton(parent)
{
    setFlat(true);
    setAutoFillBackground(true);
    setImageColor(color);
    connect(this, SIGNAL(clicked(bool)), SLOT(onClicked(bool)));
}

void ImageColor::setImageColor(const QColor& color)
{
    this->color = color;
    QPalette palette;
    palette.setBrush(QPalette::Button, QBrush(color));
    setPalette(palette);
    fillImage();

    emit imageChanged(objectName(), getImage());
}

void ImageColor::setImageSize(const QSize& size)
{
    if (image.size() == size)
        return;
    image = QImage(size, QImage::Format_RGB888);
    fillImage();

    emit imageChanged(objectName(), getImage());
}

const QImage ImageColor::getImage()
{
    return image;
}

void ImageColor::onClicked(bool)
{
    QColor newColor = QColorDialog::getColor(color, this, tr("Image Color"));
    if (!newColor.isValid())
        return;
    setImageColor(newColor);
}

void ImageColor::fillImage()
{
    if (image.isNull())
        return;

    QSize size = image.size();
    QRectF rect(0, 0, size.width(), size.height());

    QPainter painter(&image);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // Fill background
    painter.fillRect(rect, color);

    // Draw name centered, pick color to contrast with background
    painter.setPen(color.lightnessF() < 0.5 ? Qt::white : Qt::black);
    painter.setFont(QFont("Arial", 30));
    painter.drawText(rect, Qt::AlignCenter, objectName());

    // Outline edges
    const int penWidth = 10;
    QPen pen(color.lightnessF() < 0.5 ? color.lighter() : color.darker());
    pen.setWidth(penWidth);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);
    painter.drawRect(rect.adjusted(penWidth/2, penWidth/2, -penWidth/2, -penWidth/2));

    painter.end();
}
