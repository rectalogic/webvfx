#ifndef MOTIONBOX_CHROMIX_MIXRENDER_H_
#define MOTIONBOX_CHROMIX_MIXRENDER_H_

#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebKit/chromium/public/WebViewClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrameClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLError.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSize.h>
#include <skia/ext/platform_canvas.h>

namespace Chromix {
    
class MixRender : public WebKit::WebFrameClient,
                  public WebKit::WebViewClient
{
public:
    MixRender(int width, int height);
    virtual ~MixRender();

    bool loadURL(const std::string& url);
    const SkBitmap& render();//XXX need keyed video frames and current time

protected:
    // WebKit::WebViewClient
    virtual void didStopLoading();
    // WebKit::WebFrameClient
    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    
private:
    WebKit::WebView *webView;
    WebKit::WebSize size;
    skia::PlatformCanvas skiaCanvas;
    bool inMessageLoop;
    bool isLoadFinished;
};
    
}

#endif
