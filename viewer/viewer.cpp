// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QLabel>
#include <QList>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringBuilder>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QWebView>
#include <QtGlobal>
#include <webvfx/effects.h>
#include <webvfx/image.h>
#include <webvfx/parameters.h>
#include <webvfx/webvfx.h>
#include <webvfx/qml_content.h>
#include <webvfx/web_content.h>
#include "image_color.h"
#include "viewer.h"


// Expose parameter name/value pairs from the table to the page content
class ViewerParameters : public WebVfx::Parameters
{
public:
    ViewerParameters(QTableWidget* tableWidget) : tableWidget(tableWidget) {}

    double getNumberParameter(const QString& name) {
        QString value = findValue(name);
        return value.toDouble();
    }

    QString getStringParameter(const QString& name) {
        return findValue(name);
    }

private:
    QString findValue(const QString& name) {
        QList<QTableWidgetItem*> itemList = tableWidget->findItems(name, Qt::MatchFixedString|Qt::MatchCaseSensitive);
        foreach (const QTableWidgetItem* item, itemList) {
            // If the string matches column 0 (Name), then return column 1 (Value)
            if (item->column() == 0) {
                QTableWidgetItem* valueItem = tableWidget->item(item->row(), 1);
                if (valueItem)
                    return valueItem->text();
            }
        }
        return QString();
    }

    QTableWidget* tableWidget;
};

/////////////////

class ViewerLogger : public WebVfx::Logger
{
public:
    ViewerLogger(QPlainTextEdit* logText) : logText(logText) {}
    void log(const QString& msg) {
        logText->appendPlainText(msg);
    }

private:
    QPlainTextEdit* logText;
};

/////////////////

Viewer::Viewer(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    WebVfx::setLogger(new ViewerLogger(logTextEdit));

    // Time display
    timeSpinBox = new QDoubleSpinBox(statusBar());
    timeSpinBox->setDecimals(4);
    timeSpinBox->setSingleStep(0.01);
    timeSpinBox->setMaximum(1.0);
    timeSpinBox->setValue(sliderTimeValue(timeSlider->value()));
    statusBar()->addPermanentWidget(timeSpinBox);
    connect(timeSpinBox, SIGNAL(valueChanged(double)),
            SLOT(onTimeSpinBoxValueChanged(double)));

    // Size display
    sizeLabel = new QLabel(statusBar());
    statusBar()->addPermanentWidget(sizeLabel);

    handleResize();
}

void Viewer::on_actionOpenHtml_triggered(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open HTML"), QString(),
                                                    tr("HTML Files (*.html)"));
    if (fileName.isNull())
        return;
    if (!loadHtml(fileName))
        statusBar()->showMessage(tr("Load failed"), 2000);
    else
        setWindowFilePath(fileName);
}

void Viewer::on_actionOpenQml_triggered(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open QML"), QString(),
                                                    tr("QML Files (*.qml)"));
    if (fileName.isNull())
        return;
    if (!loadQml(fileName))
        statusBar()->showMessage(tr("Load failed"), 2000);
    else
        setWindowFilePath(fileName);
}

void Viewer::on_actionReload_triggered(bool)
{
    QString fileName(windowFilePath());
    if (fileName.endsWith(".qml", Qt::CaseInsensitive))
        loadQml(fileName);
    else if (fileName.endsWith(".html", Qt::CaseInsensitive))
        loadHtml(fileName);
}

void Viewer::on_resizeButton_clicked()
{
    handleResize();
}

void Viewer::handleResize()
{
    int width = widthSpinBox->value();
    int height = heightSpinBox->value();
    scrollArea->widget()->resize(width, height);
    sizeLabel->setText(QString::number(width) % QLatin1Literal("x") %
                       QString::number(height));

    // Iterate over ImageColor widgets in table and change their sizes
    QSize size(width, height);
    int rowCount = imagesTable->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ImageColor* imageColor = static_cast<ImageColor*>(imagesTable->cellWidget(i, 1));
        if (imageColor)
            imageColor->setImageSize(size);
    }
}

void Viewer::on_timeSlider_valueChanged(int value)
{
    timeSpinBox->setValue(sliderTimeValue(value));
}

void Viewer::onTimeSpinBoxValueChanged(double time)
{
    if (content) {
        content->renderContent(time, 0);
    }
    timeSlider->blockSignals(true);
    timeSlider->setValue(time * timeSlider->maximum());
    timeSlider->blockSignals(false);
}

void Viewer::on_addParameterButton_clicked()
{
    int row = parametersTable->currentRow();
    parametersTable->insertRow(row >= 0 ? row : 0);
}

void Viewer::on_deleteParameterButton_clicked()
{
    int row = parametersTable->currentRow();
    if (row >= 0)
        parametersTable->removeRow(row);
}

double Viewer::sliderTimeValue(int value)
{
    return value / (double)timeSlider->maximum();
}

bool Viewer::loadHtml(const QString& fileName)
{
    logTextEdit->clear();

    QSize size = scrollArea->widget()->size();

    // Set QWebView as direct widget of QScrollArea,
    // otherwise it creates an intermediate QWidget which messes up resizing.
    QWebView* webView = new QWebView(scrollArea);
    scrollArea->setWidget(webView);
    webView->resize(size);

    WebVfx::WebContent* webContent =
        new WebVfx::WebContent(webView, size,
                               new ViewerParameters(parametersTable));

    // User can right-click to open WebInspector on the page
    webContent->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,
                                         true);
    webView->setPage(webContent);
    content = webContent;

    bool result = webContent->loadContent(fileName);

    setupImages(size);

    content->renderContent(timeSpinBox->value(), 0);

    return result;
}

bool Viewer::loadQml(const QString& fileName)
{
    logTextEdit->clear();

    // Set QmlContent as direct widget of QScrollArea,
    // otherwise it creates an intermediate QWidget which messes up resizing.
    WebVfx::QmlContent* qmlContent =
        new WebVfx::QmlContent(scrollArea, scrollArea->widget()->size(),
                               new ViewerParameters(parametersTable));
    scrollArea->setWidget(qmlContent);
    content = qmlContent;

    bool result = qmlContent->loadContent(fileName);

    setupImages(qmlContent->size());

    content->renderContent(timeSpinBox->value(), 0);

    return result;
}

void Viewer::setupImages(const QSize& size)
{
    imagesTable->setRowCount(0);
    int row = 0;
    WebVfx::Effects::ImageTypeMapIterator it(content->getImageTypeMap());
    while (it.hasNext()) {
        it.next();

        imagesTable->insertRow(row);

        QString imageName(it.key());

        // Image name in column 0
        QTableWidgetItem* item = new QTableWidgetItem(imageName);
        item->setFlags(Qt::NoItemFlags);
        imagesTable->setItem(row, 0, item);

        // Image color swatch in column 1
        ImageColor* imageColor = new ImageColor();
        imageColor->setImageSize(size);
        imageColor->setObjectName(imageName);
        connect(imageColor, SIGNAL(imageChanged(QString,WebVfx::Image)), SLOT(onImageChanged(QString,WebVfx::Image)));
        // Set color here so signal fires
        imageColor->setImageColor(QColor::fromHsv(qrand() % 360, 200, 230));
        imagesTable->setCellWidget(row, 1, imageColor);

        // Type name in column 2
        QString typeName;
        switch (it.value()) {
            case WebVfx::Effects::SourceImageType:
                typeName = tr("Source");
                break;
            case WebVfx::Effects::TargetImageType:
                typeName = tr("Target");
                break;
            case WebVfx::Effects::ExtraImageType:
                typeName = tr("Extra");
                break;
        }
        item = new QTableWidgetItem(typeName);
        item->setFlags(Qt::NoItemFlags);
        imagesTable->setItem(row, 2, item);

        row++;
    }
}

void Viewer::onImageChanged(const QString& name, WebVfx::Image& image)
{
    if (!content)
        return;
    content->setImage(name, &image);
}
