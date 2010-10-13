#include "Loader.h"

#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebKit/chromium/public/WebConsoleMessage.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLError.h>
#include <base/message_loop.h>

Chromix::Loader::Loader() :
    inMessageLoop(false),
    isLoadFinished(false),
    didLoadSucceed(false),
    logger(NULL),
    logData(NULL)
{
}

void Chromix::Loader::setLogger(LogCallback logger, const void* data) {
    this->logger = logger;
    this->logData = data;
}

bool Chromix::Loader::loadURL(WebKit::WebView *webView, const std::string& url) {
    isLoadFinished = false;
    didLoadSucceed = false;

    WebKit::WebFrame *webFrame = webView->mainFrame();
    GURL gurl(url);
    if (!gurl.is_valid())
        return didLoadSucceed;
    webFrame->loadRequest(WebKit::WebURLRequest(gurl));
    while (!isLoadFinished) {
        inMessageLoop = true;
        MessageLoop::current()->Run();
        inMessageLoop = false;
    }

    //XXX Probably need to do this periodically when rendering video frames
    //XXXcrashes webFrame->collectGarbage();

    return didLoadSucceed;
}


void Chromix::Loader::didStopLoading() {
    // This is called even after load failure, so don't reset flags if we already failed.
    if (!isLoadFinished) {
        isLoadFinished = true;
        didLoadSucceed = true;
    }
    if (inMessageLoop)
        MessageLoop::current()->Quit();
}

void Chromix::Loader::didAddMessageToConsole(const WebKit::WebConsoleMessage& message, const WebKit::WebString& sourceName, unsigned sourceLine) {
    if (logger)
        logger(std::string(message.text.utf8()), logData);
}

void Chromix::Loader::didFailProvisionalLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    handlLoadFailure(error);
}

void Chromix::Loader::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    handlLoadFailure(error);
}

void Chromix::Loader::handlLoadFailure(const WebKit::WebURLError& error) {
    isLoadFinished = true;
    didLoadSucceed = false;
    if (inMessageLoop)
        MessageLoop::current()->Quit();
}
