// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QList>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringBuilder>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QWebSettings>
#include <QtGlobal>
#include <webvfx/effects.h>
#include <webvfx/image.h>
#include <webvfx/parameters.h>
#include <webvfx/webvfx.h>
#include <webvfx/qml_content.h>
#include <webvfx/web_content.h>
#include "image_color.h"
#include "render_dialog.h"
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

Viewer::Viewer()
    : QMainWindow(0)
    , sizeLabel(0)
    , timeSpinBox(0)
    , content(0)
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

    setContentUIEnabled(false);

    handleResize();
}

void Viewer::setContentUIEnabled(bool enable)
{
    timeSpinBox->setEnabled(enable);
    timeSlider->setEnabled(enable);
    actionReload->setEnabled(enable);
    actionRenderImage->setEnabled(enable);
}

void Viewer::onContentLoadFinished(bool result)
{
    if (result) {
        setupImages(scrollArea->widget()->size());
        content->renderContent(timeSpinBox->value(), 0);
    }
    else {
        statusBar()->showMessage(tr("Load failed"), 2000);
        setWindowFilePath("");
    }
    setContentUIEnabled(result);
}

void Viewer::on_actionOpen_triggered(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"),
                                                    QString(),
                                                    tr("WebVfx Files (*.html *.htm *.qml)"));
    loadFile(fileName);
}

void Viewer::on_actionReload_triggered(bool)
{
    content->reload();
}

void Viewer::on_actionRenderImage_triggered(bool)
{
    QImage image(scrollArea->widget()->size(), QImage::Format_RGB888);
    WebVfx::Image renderImage(image.bits(), image.width(), image.height(),
                              image.byteCount());
    content->renderContent(timeSpinBox->value(), &renderImage);
    RenderDialog* dialog = new RenderDialog(this);
    dialog->setImage(image);
    dialog->show();
}

void Viewer::on_resizeButton_clicked()
{
    handleResize();
}

void Viewer::loadFile(const QString& fileName)
{
    if (fileName.isNull())
        return;
    createContent(fileName);
    setWindowFilePath(fileName);
}

void Viewer::handleResize()
{
    int width = widthSpinBox->value();
    int height = heightSpinBox->value();
    scrollArea->widget()->resize(width, height);
    if (content)
        content->setContentSize(QSize(width, height));
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

void Viewer::setImagesOnContent()
{
    if (!content)
        return;
    int rowCount = imagesTable->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ImageColor* imageColor = static_cast<ImageColor*>(imagesTable->cellWidget(i, 1));
        if (imageColor) {
            WebVfx::Image image(imageColor->getImage());
            content->setImage(imageColor->objectName(), &image);
        }
    }
}

void Viewer::on_timeSlider_valueChanged(int value)
{
    timeSpinBox->setValue(sliderTimeValue(value));
}

void Viewer::onTimeSpinBoxValueChanged(double time)
{
    if (content) {
        setImagesOnContent();
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

void Viewer::createContent(const QString& fileName)
{
    if (fileName.endsWith(".qml", Qt::CaseInsensitive)) {
        WebVfx::QmlContent* qmlContent =
            new WebVfx::QmlContent(scrollArea->widget()->size(),
                                   new ViewerParameters(parametersTable));
        content = qmlContent;
        connect(qmlContent, SIGNAL(contentLoadFinished(bool)), SLOT(onContentLoadFinished(bool)));
    }
    else if (fileName.endsWith(".html", Qt::CaseInsensitive) || fileName.endsWith(".htm", Qt::CaseInsensitive)){
        WebVfx::WebContent* webContent =
            new WebVfx::WebContent(scrollArea->widget()->size(),
                                   new ViewerParameters(parametersTable));
        // User can right-click to open WebInspector on the page
        webContent->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        content = webContent;
        connect(webContent, SIGNAL(contentLoadFinished(bool)), SLOT(onContentLoadFinished(bool)));
    }

    QWidget* view = content->createView(scrollArea);
    view->resize(scrollArea->widget()->size());

    // Set content as direct widget of QScrollArea,
    // otherwise it creates an intermediate QWidget which messes up resizing.
    // setWidget will destroy the old view.
    scrollArea->setWidget(view);

    logTextEdit->clear();

    setContentUIEnabled(true);
    content->loadContent(QUrl::fromLocalFile(QFileInfo(fileName).absoluteFilePath()));
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
        connect(imageColor, SIGNAL(imageChanged(QString,WebVfx::Image)),
                SLOT(onImageChanged(QString,WebVfx::Image)));
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

void Viewer::onImageChanged(const QString& name, WebVfx::Image image)
{
    if (!content)
        return;
    content->setImage(name, &image);
}
