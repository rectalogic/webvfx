// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDER_DIALOG_H
#define RENDER_DIALOG_H

#include <QDialog>
#include "ui_render_dialog.h"

class QImage;
class QLabel;

class RenderDialog : public QDialog, private Ui::RenderDialog
{
    Q_OBJECT
public:
    RenderDialog(QWidget* parent=0);
    void setImage(const QImage& image);
};

#endif

