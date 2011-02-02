#include <sstream>
#include <QWebFrame>
#include <QEventLoop>
#include "chromix/web_page.h"
#include "chromix/logger.h"

Chromix::WebPage::WebPage(QObject *parent)
    : QWebPage(parent)
    , syncLoop(0)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(loadSyncFinished(bool)));
}

void Chromix::WebPage::javaScriptAlert(QWebFrame *originatingFrame, const QString &msg)
{
    Q_UNUSED(originatingFrame);
    Chromix::log(std::string("JavaScript alert: ") + msg.toStdString());
}

void Chromix::WebPage::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID)
{
    std::ostringstream oss;
    if (!sourceID.isEmpty())
        oss << sourceID.toStdString() << ":" << lineNumber << " ";
    oss << message.toStdString();
    Chromix::log(oss.str());
}

bool Chromix::WebPage::shouldInterruptJavaScript()
{
    return false;
}

void Chromix::WebPage::loadSyncFinished(bool result)
{
    //XXX we should also wait for JS in loaded page to call "chromix.setRenderCallback"
    //XXX exit loop when page is loaded and callback is set (unless load result fails)
    if (syncLoop)
        syncLoop->exit(result);
}

bool Chromix::WebPage::loadSync(const QUrl& url)
{
    if (syncLoop) {
        Chromix::log("loadSync recursive call detected");
        return false;
    }

    mainFrame()->load(url);

    // Run a nested event loop which loadSyncFinished will end,
    // returning the result code here.
    // http://wiki.forum.nokia.com/index.php/How_to_wait_synchronously_for_a_Signal_in_Qt
    // http://qt.gitorious.org/qt/qt/blobs/4.7/src/gui/dialogs/qdialog.cpp#line549
    QEventLoop loop;
    syncLoop = &loop;
    bool result = loop.exec();
    syncLoop = 0;
    return result;
}
