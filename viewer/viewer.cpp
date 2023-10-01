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
#include <QUrlQuery>

Viewer::Viewer()
    : QMainWindow(0)
    , sizeLabel(0)
    , timeSpinBox(0)
    , contentPipe(0)
{
    setupUi(this);

    // XXX capture stderr and send to logTextEdit
    // XXX WebVfx::setLogger(new ViewerLogger(logTextEdit));

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
    if (!contentPipe)
        return;

    int rowCount = imagesTable->rowCount();
    QList<QImage> sourceImages(rowCount);
    for (int i = 0; i < rowCount; i++) {
        ImageColor* imageColor = static_cast<ImageColor*>(imagesTable->cellWidget(i, 1));
        if (imageColor) {
            sourceImages[i] = imageColor->getImage();
        }
    }

    if (renderImage.size() != scrollArea->widget()->size())
        renderImage = QImage(scrollArea->widget()->size(), QImage::Format_RGBA8888);
    contentPipe->renderContent(timeSpinBox->value(), sourceImages, renderImage);
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

    QUrlQuery query;
    for (int row = 0; row < parametersTable->rowCount(); row++) {
        query.addQueryItem(
            QUrl::toPercentEncoding(parametersTable->item(row, 0)->text()),
            QUrl::toPercentEncoding(parametersTable->item(row, 1)->text()));
    }
    QUrl qmlUrl(QUrl::fromLocalFile(QFileInfo(fileName).absoluteFilePath()));
    qmlUrl.setQuery(query);

    ContentPipe* qmlContentPipe = new ContentPipe(qmlUrl, this);
    delete contentPipe;
    contentPipe = qmlContentPipe;
    imageLabel = new QLabel(scrollArea);

    // Set imageLabel as direct widget of QScrollArea,
    // otherwise it creates an intermediate QWidget which messes up resizing.
    // setWidget will destroy the old view.
    scrollArea->setWidget(imageLabel);
    scrollArea->widget()->resize(size);
    imageLabel->show();

    logTextEdit->clear();

    setContentUIEnabled(false); // XXX need UI enabled all the time now?

    setupImages(scrollArea->widget()->size());
    renderContent();
}

void Viewer::setupImages(const QSize& size)
{
    int imageCount = 0; // XXX need a count, make frameserver return count
    imagesTable->setRowCount(imageCount);
    for (qsizetype row = 0; row < imageCount; ++row) {
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
    if (!contentPipe)
        return;
    renderContent();
}
