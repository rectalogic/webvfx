// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QImage>
#include <QPixmap>
#include <QLabel>
#include "render_dialog.h"

RenderDialog::RenderDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

void RenderDialog::setImage(const QImage& image)
{
    imageLabel->setPixmap(QPixmap::fromImage(image));
}
