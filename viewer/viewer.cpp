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
#include <webvfx/webvfx.h>
#include <webvfx/web_effects.h>
#include <webvfx/web_page.h>
#include <webvfx/web_parameters.h>
#include "color_swatch.h"
#include "viewer.h"


// Expose parameter name/value pairs from the table to the page content
class ViewerParameters : public WebVFX::WebParameters
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

class ViewerLogger : public WebVFX::WebLogger
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

    WebVFX::setLogger(new ViewerLogger(logTextEdit));

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
}

void Viewer::on_timeSlider_valueChanged(int value)
{
    double time = sliderTimeValue(value);
    if (webPage) {
        // Just ignore the returned WebImage
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
    //XXX query for image data - create QImages and QPainter color and text into them
    //XXX need to do this each time resized - rebuild images at new size
    webPage = new WebVFX::WebPage(webView, webView->size(), new ViewerParameters(parametersTable));
    // User can right-click to open WebInspector on the page
    webPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    webView->setPage(webPage);
    bool result = webPage->loadSync(url);

    timeSlider->setValue(0);
    logTextEdit->clear();

    setupImages();

    // Install WebInspector action on tool button
    inspectorButton->setDefaultAction(webPage->action(QWebPage::InspectElement));

    return result;
}

void Viewer::setupImages()
{
    imagesTable->setRowCount(0);
    const WebVFX::WebEffects::ImageTypeMap& imageMap = webPage->getImageTypeMap();
    int row = 0;
    for (WebVFX::WebEffects::ImageTypeMap::const_iterator it = imageMap.begin();
         it != imageMap.end(); it++) {
        imagesTable->insertRow(row);

        // Image name in column 0
        QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(it->first));
        item->setFlags(Qt::NoItemFlags);
        imagesTable->setItem(row, 0, item);

        // Image color swatch in column 1
        ColorSwatch* colorSwatch = new ColorSwatch();
        //XXX generate random color
        //XXX connect to colorChanged signal and regenerate image when fired
        imagesTable->setCellWidget(row, 1, colorSwatch);

        // Type name in column 2
        QString typeName;
        switch (it->second) {
            case WebVFX::WebEffects::SourceImageType:
                typeName = tr("Source");
                break;
            case WebVFX::WebEffects::TargetImageType:
                typeName = tr("Target");
                break;
            case WebVFX::WebEffects::ExtraImageType:
                typeName = tr("Extra");
                break;
        }
        item = new QTableWidgetItem(typeName);
        item->setFlags(Qt::NoItemFlags);
        imagesTable->setItem(row, 2, item);

        row++;
    }
    //XXX need to create QImages too
}
