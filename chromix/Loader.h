#ifndef MOTIONBOX_CHROMIX_LOADER_H_
#define MOTIONBOX_CHROMIX_LOADER_H_

#include <third_party/WebKit/WebKit/chromium/public/WebViewClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrameClient.h>

namespace WebKit {
    class WebFrame;
    class WebURLError;
    class WebView;
    class WebConsoleMessage;
}

namespace Chromix {

typedef void(*LogCallback)(string16 const&, const void* data);

class Loader : public WebKit::WebFrameClient,
               public WebKit::WebViewClient
{
public:
    Loader();
    virtual ~Loader() {};

    void setLogger(LogCallback logger, const void* data);
    bool loadURL(WebKit::WebView *webView, string16 const& url);

protected:
    // WebKit::WebViewClient
    virtual void didStopLoading();
    virtual void didAddMessageToConsole(const WebKit::WebConsoleMessage&, const WebKit::WebString& sourceName, unsigned sourceLine);
    // WebKit::WebFrameClient
    virtual void didFailProvisionalLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    
    void handlLoadFailure(const WebKit::WebURLError&);
    
private:
    bool inMessageLoop;
    bool isLoadFinished;
    bool didLoadSucceed;

    LogCallback logger;
    const void* logData;
};

}

#endif

