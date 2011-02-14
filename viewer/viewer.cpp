// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QDir>
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
#include <webvfx/web_page.h>
#include "image_color.h"
#include "viewer.h"


// Expose parameter name/value pairs from the table to the page content
class ViewerParameters : public WebVfx::Parameters
{
public:
    ViewerParameters(QTableWidget* tableWidget) : tableWidget(tableWidget) {}

    double getNumberParameter(const std::string& name) {
        QString value = findValue(name);
        return value.toDouble();
    }

    std::string getStringParameter(const std::string& name) {
        QString value = findValue(name);
        return value.toStdString();
    }

private:
    QString findValue(const std::string& name) {
        QList<QTableWidgetItem*> itemList = tableWidget->findItems(QString::fromStdString(name), Qt::MatchFixedString|Qt::MatchCaseSensitive);
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
    void log(const std::string& msg) {
        logText->appendPlainText(QString::fromStdString(msg));
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

    // Set QWebView as direct widget of QScrollArea,
    // otherwise it creates an intermediate QWidget which messes up resizing.
    webView = new QWebView(scrollArea);
    scrollArea->setWidget(webView);

    // Time display
    timeLabel = new QLabel(statusBar());
    timeLabel->setNum(sliderTimeValue(timeSlider->value()));
    statusBar()->addPermanentWidget(timeLabel);

    // Size display
    sizeLabel = new QLabel(statusBar());
    statusBar()->addPermanentWidget(sizeLabel);

    on_resizeButton_clicked();
}

void Viewer::on_actionOpen_triggered(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Page"), QDir::homePath(), tr("HTML Files (*.html)"));
    if (fileName.isNull())
        return;
    if (!loadPage(QUrl::fromLocalFile(fileName)))
        statusBar()->showMessage(tr("Load failed"), 2000);
    else
        setWindowFilePath(fileName);
}

void Viewer::on_resizeButton_clicked()
{
    int width = widthSpinBox->value();
    int height = heightSpinBox->value();
    scrollArea->widget()->resize(width, height);
    sizeLabel->setText(QString::number(width) % QLatin1Literal("x") % QString::number(height));

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
    double time = sliderTimeValue(value);
    if (webPage) {
        // Just ignore the returned Image
        webPage->render(time);
    }
    timeLabel->setNum(time);
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


bool Viewer::loadPage(const QUrl& url)
{
    logTextEdit->clear();

    webPage = new WebVfx::WebPage(webView, webView->size(), new ViewerParameters(parametersTable));
    // User can right-click to open WebInspector on the page
    webPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    // Install WebInspector action on tool button
    inspectorButton->setDefaultAction(webPage->action(QWebPage::InspectElement));
    webView->setPage(webPage);

    bool result = webPage->loadSync(url);

    timeSlider->setValue(0);

    setupImages(webView->size());

    return result;
}

void Viewer::setupImages(const QSize& size)
{
    imagesTable->setRowCount(0);
    const WebVfx::Effects::ImageTypeMap& imageMap = webPage->getImageTypeMap();
    int row = 0;
    for (WebVfx::Effects::ImageTypeMap::const_iterator it = imageMap.begin();
         it != imageMap.end(); it++) {
        imagesTable->insertRow(row);

        QString imageName(QString::fromStdString(it->first));

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
        switch (it->second) {
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

void Viewer::onImageChanged(const QString& name, const WebVfx::Image& image)
{
    if (!webPage)
        return;
    WebVfx::Image targetImage = webPage->getImage(name, QSize(image.width(), image.height()));
    targetImage.copyPixelsFrom(image);
}
