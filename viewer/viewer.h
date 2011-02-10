#ifndef VIEWER_FORM_H
#define VIEWER_FORM_H

#include <QMainWindow>
#include "ui_viewer.h"

namespace WebVFX
{
    class WebPage;
}

class QLabel;

class Viewer : public QMainWindow, private Ui::Viewer
{
    Q_OBJECT

public:
    Viewer(QWidget *parent = 0);

private slots:
    void on_actionOpen_triggered(bool);
    void on_resizeButton_clicked();
    void on_addParameterButton_clicked();
    void on_deleteParameterButton_clicked();
private:
    bool loadPage(const QUrl& url);
    QLabel* sizeLabel;
    WebVFX::WebPage* webPage;
};

#endif

