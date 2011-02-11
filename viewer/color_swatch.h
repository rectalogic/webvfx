// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COLOR_SWATCH_H
#define COLOR_SWATCH_H

#include <QPushButton>
#include <QColor>

class ColorSwatch : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    ColorSwatch(QWidget *parent = 0);

signals:
    void colorChanged(const QColor& color);

private:
    const QColor& color() { return color_; }
    void setColor(const QColor& color);

private slots:
    void onClicked(bool);

private:
    QColor color_;
};

#endif

