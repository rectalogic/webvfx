// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/loader.h"
#include "chromix/delegate.h"

#include <third_party/WebKit/Source/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebURLError.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebConsoleMessage.h>
#include <base/string16.h>
#include <base/message_loop.h>

#include <ostringstream>

Chromix::Loader::Loader(Delegate* delegate) :
    delegate(delegate),
    inMessageLoop(false),
    isLoadFinished(false),
    didLoadSucceed(false)
{
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

void Chromix::Loader::didAddMessageToConsole(const WebKit::WebConsoleMessage& message, const WebKit::WebString& sourceName, unsigned sourceLine) {
    if (delegate) {
        std::ostringstream oss;
        oss << std::string(sourceName.utf8()) << ":" << sourceLine << " - " << std::string(message.text.utf8());
        delegate->logMessage(oss.str());
    }
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

void Chromix::Loader::didFailProvisionalLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    handlLoadFailure(error);
}

void Chromix::Loader::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    handlLoadFailure(error);
}

void Chromix::Loader::handlLoadFailure(const WebKit::WebURLError& error) {
    if (delegate)
        delegate->logMessage(std::string("Failed to load ") + std::string(error.unreachableURL.spec()));
    isLoadFinished = true;
    didLoadSucceed = false;
    if (inMessageLoop)
        MessageLoop::current()->Quit();
}
