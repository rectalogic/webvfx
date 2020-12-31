// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWER_H
#define VIEWER_H

#include <QMainWindow>
#include <QMessageLogContext>
#include <QtGlobal>
#include <webvfx/image.h>
#include "ui_viewer.h"

namespace WebVfx
{
    class Content;
}

class QDoubleSpinBox;
class QLabel;

class Viewer : public QMainWindow, private Ui::Viewer
{
    Q_OBJECT

public:
    Viewer();
    ~Viewer();
    void loadFile(const QString& fileName);
    void messageHandler(QtMsgType, const QMessageLogContext &, const QString &);
private slots:
    void on_actionOpen_triggered(bool);
    void onContentLoadFinished(bool);
    void on_actionReload_triggered(bool);
    void on_resizeButton_clicked();
    void on_timeSlider_valueChanged(int);
    void onTimeSpinBoxValueChanged(double);
    void on_addParameterButton_clicked();
    void on_deleteParameterButton_clicked();
    void onImageChanged(const QString&, WebVfx::Image);

private:
    void createContent(const QString& fileName);
    void renderContent();
    void setContentUIEnabled(bool enable);
    void handleResize();
    void setImagesOnContent();
    void setupImages(const QSize& size);
    double sliderTimeValue(int value);
    QLabel* sizeLabel;
    QDoubleSpinBox* timeSpinBox;
    QLabel* imageLabel;
    WebVfx::Content* content;
};

#endif

