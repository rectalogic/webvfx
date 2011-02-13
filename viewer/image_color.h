// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IMAGE_COLOR_H
#define IMAGE_COLOR_H

#include <QPushButton>
#include <QColor>
#include <QSize>

namespace WebVFX {
    class Image;
}

class ImageColor : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor imageColor READ imageColor WRITE setImageColor)
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize)

public:
    ImageColor(QWidget *parent = 0);

    const QColor imageColor() { return color; }
    void setImageColor(const QColor& color);
    const QSize imageSize() { return image.size(); }
    void setImageSize(const QSize& size);

signals:
    void imageChanged(const QString& name, const WebVFX::Image& image);

private slots:
    void onClicked(bool);

private:
    void fillImage();
    const WebVFX::Image getImage();
    QColor color;
    QImage image;
};

#endif

