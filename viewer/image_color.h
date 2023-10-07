// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QColor> // for QColor
#include <QImage> // for QImage
#include <QObject> // for Q_PROPERTY, Q_OBJECT, signals, slots
#include <QPushButton> // for QPushButton
#include <QSize> // for QSize
#include <QString> // for QString
class QWidget;

class ImageColor : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor imageColor READ imageColor WRITE setImageColor)
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize)

public:
    ImageColor(QWidget* parent = 0);

    const QColor imageColor() { return color; }
    void setImageColor(const QColor& color);
    const QSize imageSize() { return image.size(); }
    void setImageSize(const QSize& size);
    const QImage getImage();

signals:
    void imageChanged(const QString& name, QImage image);

private slots:
    void onClicked(bool);

private:
    void fillImage();
    QColor color;
    QImage image;
};
