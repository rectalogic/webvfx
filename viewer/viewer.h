// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "ui_viewer.h"
#include <QImage>
#include <QMainWindow>
#include <QMessageLogContext>
#include <QtGlobal>
#include <vfxpipe.h>

namespace WebVfx {
class QmlContent;
}

class QDoubleSpinBox;
class QLabel;

class Viewer : public QMainWindow, private Ui::Viewer {
    Q_OBJECT

public:
    Viewer();
    ~Viewer();
    void loadFile(const QString& fileName);
    void messageHandler(QtMsgType, const QMessageLogContext&, const QString&);
private slots:
    void on_actionOpen_triggered(bool);
    void on_actionReload_triggered(bool);
    void on_resizeButton_clicked();
    void on_timeSlider_valueChanged(int);
    void onTimeSpinBoxValueChanged(double);
    void on_addParameterButton_clicked();
    void on_deleteParameterButton_clicked();
    void onImageChanged(const QString&, QImage);

private:
    void createContent(const QString& fileName);
    void renderContent();
    void setContentUIEnabled(bool enable);
    void handleResize();
    void setupImages(const QSize& size);
    double sliderTimeValue(int value);
    QLabel* sizeLabel;
    QDoubleSpinBox* timeSpinBox;
    QLabel* imageLabel;
    QImage renderImage;
    VfxPipe::FrameServer* frameServer;
};
