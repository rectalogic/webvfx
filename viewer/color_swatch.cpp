// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QBrush>
#include <QColorDialog>
#include <QPalette>
#include "color_swatch.h"

ColorSwatch::ColorSwatch(QWidget* parent)
    : QPushButton(parent)
{
    setFlat(true);
    setAutoFillBackground(true);
    setColor(color_);
    connect(this, SIGNAL(clicked(bool)), SLOT(onClicked(bool)));
}

void ColorSwatch::setColor(const QColor& color)
{
    color_ = color;
    QPalette palette;
    palette.setBrush(QPalette::Button, QBrush(color));
    setPalette(palette);
}

void ColorSwatch::onClicked(bool)
{
    QColor newColor = QColorDialog::getColor(color_, this, tr("Image Color"));
    if (!newColor.isValid())
        return;
    setColor(newColor);
    emit colorChanged(newColor);
}
