// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_LOADER_H_
#define CHROMIX_LOADER_H_


#include <string>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebViewClient.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebFrameClient.h>

namespace WebKit {
    class WebFrame;
    class WebURLError;
    class WebView;
    class WebConsoleMessage;
}

namespace Chromix {

class Delegate;

class Loader : public WebKit::WebFrameClient,
               public WebKit::WebViewClient
{
public:
    Loader(Delegate* delegate);
    virtual ~Loader() {};

    bool loadURL(WebKit::WebView *webView, const std::string& url);

protected:
    // WebKit::WebViewClient
    virtual void didStopLoading();
    virtual void didAddMessageToConsole(const WebKit::WebConsoleMessage&, const WebKit::WebString& sourceName, unsigned sourceLine);
    // WebKit::WebFrameClient
    virtual void didFailProvisionalLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);

    void handlLoadFailure(const WebKit::WebURLError&);

private:
    Delegate* delegate;
    bool inMessageLoop;
    bool isLoadFinished;
    bool didLoadSucceed;
};

}

#endif

