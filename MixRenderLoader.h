#ifndef MOTIONBOX_CHROMIX_MIXRENDERLOADER_H_
#define MOTIONBOX_CHROMIX_MIXRENDERLOADER_H_

#include <third_party/WebKit/WebKit/chromium/public/WebViewClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrameClient.h>

namespace WebKit {
    class WebFrame;
    class WebURLError;
    class WebView;
}

namespace Chromix {
    
class MixRenderLoader : public WebKit::WebFrameClient,
                        public WebKit::WebViewClient
{
public:
    MixRenderLoader();
    virtual ~MixRenderLoader() {};
    
    bool loadURL(WebKit::WebView *webView, const std::string& url);

protected:
    // WebKit::WebViewClient
    virtual void didStopLoading();
    // WebKit::WebFrameClient
    virtual void didFailProvisionalLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    
    void handlLoadFailure(const WebKit::WebURLError&);
    
private:
    bool inMessageLoop;
    bool isLoadFinished;
    bool didLoadSucceed;
};

}

#endif

