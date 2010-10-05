#ifndef MOTIONBOX_CHROMIX_MIXRENDER_H_
#define MOTIONBOX_CHROMIX_MIXRENDER_H_

#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebKit/chromium/public/WebViewClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrameClient.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLError.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSize.h>
#include <skia/ext/platform_canvas.h>
#undef LOG //XXX
#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefPtr.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/StringHash.h>
#include <third_party/WebKit/JavaScriptCore/wtf/HashMap.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>


namespace Chromix {

class MixRender : public WebKit::WebFrameClient,
                  public WebKit::WebViewClient
{
public:
    MixRender(int width, int height);
    virtual ~MixRender();

    bool loadURL(const std::string& url);
    const SkBitmap& render();//XXX need keyed video frames and current time

    static MixRender* fromWebView(WebKit::WebView* webView);
    WTF::PassRefPtr<WebCore::ImageData> imageDataForKey(WTF::String key, unsigned int width, unsigned int height);

protected:
    // WebKit::WebViewClient
    virtual void didStopLoading();
    // WebKit::WebFrameClient
    virtual void didFailProvisionalLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);

    void handlLoadFailure(const WebKit::WebURLError&);

private:
    WebKit::WebView *webView;
    WebKit::WebSize size;
    skia::PlatformCanvas skiaCanvas;
    bool inMessageLoop;
    bool isLoadFinished;
    bool didLoadSucceed;

    typedef WTF::HashMap<WTF::String, WTF::RefPtr<WebCore::ImageData> > ImageMap;
    ImageMap imageMap;
};
    
}

#endif
