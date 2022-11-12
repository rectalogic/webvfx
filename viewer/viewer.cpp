// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "viewer.h"
#include "image_color.h"
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QList>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStringBuilder>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <webvfx/parameters.h>
#include <webvfx/qml_content.h>
#include <webvfx/webvfx.h>

// Expose parameter name/value pairs from the table to the page content
class ViewerParameters : public WebVfx::Parameters {
public:
    ViewerParameters(QTableWidget* tableWidget)
        : tableWidget(tableWidget)
    {
    }

    double getNumberParameter(const QString& name)
    {
        QString value = findValue(name);
        return value.toDouble();
    }

    QString getStringParameter(const QString& name)
    {
        return findValue(name);
    }

private:
    QString findValue(const QString& name)
    {
        QList<QTableWidgetItem*> itemList = tableWidget->findItems(name, Qt::MatchFixedString | Qt::MatchCaseSensitive);
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

class ViewerLogger : public WebVfx::Logger {
public:
    ViewerLogger(QPlainTextEdit* logText)
        : logText(logText)
    {
    }
    void log(const QString& msg)
    {
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

Viewer::~Viewer()
{
    delete content;
}

void Viewer::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString result;
    QTextStream(&result) << type << ": " << msg << " (line " << context.line << ", " << (context.function ? context.function : "") << ")\n";
    logTextEdit->appendPlainText(result);
}

void Viewer::setContentUIEnabled(bool enable)
{
    timeSpinBox->setEnabled(enable);
    timeSlider->setEnabled(enable);
    actionReload->setEnabled(enable);
}

void Viewer::onContentLoadFinished(bool result)
{
    if (result) {
        setupImages(scrollArea->widget()->size());
        renderContent();
    } else {
        statusBar()->showMessage(tr("Load failed"), 2000);
        setWindowFilePath("");
    }
    setContentUIEnabled(result);
}

void Viewer::on_actionOpen_triggered(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"),
        QString(),
        tr("WebVfx Files (*.qml)"));
    loadFile(fileName);
}

void Viewer::on_actionReload_triggered(bool)
{
    createContent(windowFilePath());
}

void Viewer::renderContent()
{
    if (!content)
        return;
    setImagesOnContent();
    QImage renderImage(scrollArea->widget()->size(), QImage::Format_RGBA8888);
    content->renderContent(timeSpinBox->value(), renderImage);
    imageLabel->setPixmap(QPixmap::fromImage(renderImage));
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
    sizeLabel->setText(QString::number(width) % QLatin1String("x") % QString::number(height));

    // Iterate over ImageColor widgets in table and change their sizes
    QSize size(width, height);
    int rowCount = imagesTable->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ImageColor* imageColor = static_cast<ImageColor*>(imagesTable->cellWidget(i, 1));
        if (imageColor)
            imageColor->setImageSize(size);
    }
    renderContent();
}

void Viewer::setImagesOnContent()
{
    if (!content)
        return;
    auto videoSinks = content->getVideoSinks();
    int rowCount = imagesTable->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ImageColor* imageColor = static_cast<ImageColor*>(imagesTable->cellWidget(i, 1));
        if (imageColor) {
            auto image = imageColor->getImage();
            QVideoFrame frame(QVideoFrameFormat(image.size(), QVideoFrameFormat::pixelFormatFromImageFormat(image.format())));
            frame.map(QVideoFrame::WriteOnly);
            memcpy(frame.bits(0), image.bits(), frame.mappedBytes(0));
            frame.unmap();
            videoSinks.at(i)->setVideoFrame(frame);
        }
    }
}

void Viewer::on_timeSlider_valueChanged(int value)
{
    timeSpinBox->setValue(sliderTimeValue(value));
    renderContent();
}

void Viewer::onTimeSpinBoxValueChanged(double time)
{
    timeSlider->blockSignals(true);
    timeSlider->setValue(time * timeSlider->maximum());
    timeSlider->blockSignals(false);
    renderContent();
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
    QSize size(scrollArea->widget()->size());
    WebVfx::QmlContent* qmlContent = new WebVfx::QmlContent(size, new ViewerParameters(parametersTable));
    delete content;
    content = qmlContent;
    imageLabel = new QLabel(scrollArea);
    connect(qmlContent, SIGNAL(contentLoadFinished(bool)), SLOT(onContentLoadFinished(bool)));

    // Set content as direct widget of QScrollArea,
    // otherwise it creates an intermediate QWidget which messes up resizing.
    // setWidget will destroy the old view.
    scrollArea->setWidget(imageLabel);
    scrollArea->widget()->resize(size);
    imageLabel->show();

    logTextEdit->clear();

    setContentUIEnabled(false);
    content->loadContent(QUrl::fromLocalFile(QFileInfo(fileName).absoluteFilePath()));
}

void Viewer::setupImages(const QSize& size)
{
    imagesTable->setRowCount(0);
    auto videoSinks = content->getVideoSinks();
    for (qsizetype row = 0; row < videoSinks.size(); ++row) {
        imagesTable->insertRow(row);

        QString imageName = QString("Image %1").arg(row);

        // Image name in column 0
        QTableWidgetItem* item = new QTableWidgetItem(imageName);
        item->setFlags(Qt::NoItemFlags);
        imagesTable->setItem(row, 0, item);

        // Image color swatch in column 1
        ImageColor* imageColor = new ImageColor();
        imageColor->setImageSize(size);
        imageColor->setObjectName(imageName);
        imageColor->setImageColor(QColor::fromHsv(QRandomGenerator::global()->generate() % 360, 200, 230));
        connect(imageColor, SIGNAL(imageChanged(QString, QImage)),
            SLOT(onImageChanged(QString, QImage)));
        imagesTable->setCellWidget(row, 1, imageColor);
    }
}

void Viewer::onImageChanged(const QString&, QImage)
{
    if (!content)
        return;
    setImagesOnContent();
    renderContent();
}
