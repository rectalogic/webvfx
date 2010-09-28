#ifndef MOTIONBOX_MIXKIT_WEBVIEWDELEGATE_H_
#define MOTIONBOX_MIXKIT_WEBVIEWDELEGATE_H_

#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebKit/chromium/public/WebViewClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrameClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLError.h>

namespace MixKit {

class WebViewDelegate :
    public WebKit::WebFrameClient,
    public WebKit::WebViewClient
{
public:
    WebViewDelegate() : loadFinished(false) {};

    // WebKit::WebViewClient

    virtual void didStopLoading();

    // WebKit::WebFrameClient

    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);

    bool isLoadFinished() { return loadFinished; };

private:
    bool loadFinished;
};

}
#endif

