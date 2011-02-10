#include <QtGui>
#include <QDir>
#include <QLabel>
#include <QList>
#include <QStringBuilder>
#include <QTableWidget>
#include <QUrl>
#include <webvfx/web_page.h>
#include <webvfx/web_parameters.h>
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

Viewer::Viewer(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    sizeLabel = new QLabel();
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
}

void Viewer::on_resizeButton_clicked()
{
    int width = widthSpinBox->value();
    int height = heightSpinBox->value();
    scrollArea->widget()->resize(width, height);
    sizeLabel->setText(QString::number(width) % QLatin1Literal("x") % QString::number(height));
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

bool Viewer::loadPage(const QUrl& url)
{
    //XXX query for image data
    webPage = new WebVFX::WebPage(webView, webView->size(), new ViewerParameters(parametersTable));
    webView->setPage(webPage);
    return webPage->loadSync(url);
}
