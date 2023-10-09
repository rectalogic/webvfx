// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "viewer.h"
#include "image_color.h" // for ImageColor
#include "vfxpipe.h" // for VideoFrame, FrameServer, VideoFrameFormat, VideoFrameFormat::PixelFormat, VideoFrameFormat::RGBA32
#include <QAction> // for QAction
#include <QColor> // for QColor
#include <QDebug> // for QDebug
#include <QDoubleSpinBox> // for QDoubleSpinBox
#include <QFileDialog> // for QFileDialog
#include <QFileInfo> // for QFileInfo
#include <QLabel> // for QLabel
#include <QLatin1String> // for QLatin1String
#include <QPixmap> // for QPixmap
#include <QPlainTextEdit> // for QPlainTextEdit
#include <QRandomGenerator> // for QRandomGenerator
#include <QScrollArea> // for QScrollArea
#include <QSize> // for QSize, operator!=
#include <QSlider> // for QSlider
#include <QSocketNotifier>
#include <QSpinBox> // for QSpinBox
#include <QStatusBar> // for QStatusBar
#include <QStringBuilder> // for operator%, QConcatenable<>::type, QStringBuilder
#include <QTableWidget> // for QTableWidget
#include <QTableWidgetItem> // for QTableWidgetItem
#include <QTextStream> // for QTextStream
#include <QUrl> // for QUrl
#include <QUrlQuery> // for QUrlQuery
#include <QWidget> // for QWidget
#include <QtCore> // for NoItemFlags, qsizetype
#include <algorithm> // for max
#include <cstddef> /* IWYU pragma: keep */ /* IWYU pragma: no_include <ext/type_traits> */ // for byte
#include <string> // for string
#include <unistd.h>
#include <vector> // for vector

void errorHandler(std::string msg)
{
    qCritical() << msg;
};

Viewer::Viewer()
    : QMainWindow(0)
    , sizeLabel(0)
    , timeSpinBox(0)
    , frameServer(0)
    , errorNotifier(0)
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

    setContentUIEnabled(true); // XXX

    handleResize();
}

Viewer::~Viewer()
{
    delete frameServer;
}

void Viewer::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString result;
    QTextStream(&result) << type << ": " << msg << " (line " << context.line << ", " << (context.function ? context.function : "") << ")\n";
    logTextEdit->appendPlainText(result);
}

void Viewer::onErrorReadyRead(QSocketDescriptor fd, QSocketNotifier::Type)
{
    char data[2048];
    ssize_t n = read(fd, data, sizeof(data) - 1);
    if (n > 0) {
        data[n] = 0;
        logTextEdit->appendPlainText(QString(data));
    }
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
    if (!frameServer)
        return;

    double time = timeSpinBox->value();
    int rowCount = imagesTable->rowCount();
    std::vector<VfxPipe::VideoFrame<const std::byte*>> sourceImages;
    for (int i = 0; i < rowCount; i++) {
        ImageColor* imageColor = static_cast<ImageColor*>(imagesTable->cellWidget(i, 1));
        if (imageColor) {
            auto image = imageColor->renderImage(time);
            sourceImages.emplace_back(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, image.width(), image.height(), reinterpret_cast<const std::byte*>(image.constBits()));
        }
    }

    if (renderImage.size() != scrollArea->widget()->size())
        renderImage = QImage(scrollArea->widget()->size(), QImage::Format_RGBA8888);
    VfxPipe::VideoFrame outputImage(VfxPipe::VideoFrameFormat::PixelFormat::RGBA32, renderImage.width(), renderImage.height(), reinterpret_cast<std::byte*>(renderImage.bits()));
    if (frameServer->renderFrame(time, sourceImages, outputImage, errorHandler)) {
        imageLabel->setPixmap(QPixmap::fromImage(renderImage));
    } else {
        delete frameServer;
        frameServer = nullptr;
        setContentUIEnabled(false);
    }
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
    timeSpinBox->blockSignals(true);
    timeSpinBox->setValue(sliderTimeValue(value));
    timeSpinBox->blockSignals(false);
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
        auto name = parametersTable->item(row, 0);
        auto value = parametersTable->item(row, 1);
        if (name && value) {
            query.addQueryItem(
                QUrl::toPercentEncoding(name->text()),
                QUrl::toPercentEncoding(value->text()));
        }
    }
    QUrl qmlUrl(QUrl::fromLocalFile(QFileInfo(fileName).absoluteFilePath()));
    qmlUrl.setQuery(query);

    VfxPipe::FrameServer* qmlFrameServer = new VfxPipe::FrameServer(qmlUrl.toString().toStdString());
    delete frameServer;
    frameServer = qmlFrameServer;
    imageLabel = new QLabel(scrollArea);

    // Set imageLabel as direct widget of QScrollArea,
    // otherwise it creates an intermediate QWidget which messes up resizing.
    // setWidget will destroy the old view.
    scrollArea->setWidget(imageLabel);
    scrollArea->widget()->resize(size);
    imageLabel->show();

    logTextEdit->clear();

    delete errorNotifier;
    int pipeReadStderr = 0;
    if (frameServer->initialize(errorHandler, size.width(), size.height(), &pipeReadStderr)) {
        errorNotifier = new QSocketNotifier(pipeReadStderr, QSocketNotifier::Read, this);
        connect(errorNotifier, &QSocketNotifier::activated, this, &Viewer::onErrorReadyRead);
        setContentUIEnabled(true);
    }

    setupImages(frameServer->getSinkCount(), scrollArea->widget()->size());
    renderContent();
}

void Viewer::setupImages(uint32_t imageCount, const QSize& size)
{
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
        connect(imageColor, SIGNAL(imageChanged()), SLOT(onImageChanged()));
        imagesTable->setCellWidget(row, 1, imageColor);
    }
}

void Viewer::onImageChanged()
{
    if (!frameServer)
        return;
    renderContent();
}
