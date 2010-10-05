#ifndef MOTIONBOX_CHROMIX_MIXRENDER_H_
#define MOTIONBOX_CHROMIX_MIXRENDER_H_

#include "MixRenderLoader.h"

#include <third_party/WebKit/WebKit/chromium/public/WebSize.h>
#include <skia/ext/platform_canvas.h>
#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefPtr.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/StringHash.h>
#include <third_party/WebKit/JavaScriptCore/wtf/HashMap.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>


namespace Chromix {

class MixRender {
public:
    MixRender(int width, int height);
    virtual ~MixRender();

    bool loadURL(const std::string& url);
    const SkBitmap& render();//XXX need keyed video frames and current time

    static MixRender* fromWebView(WebKit::WebView* webView);
    WTF::PassRefPtr<WebCore::ImageData> imageDataForKey(WTF::String key, unsigned int width, unsigned int height);


private:
    WebKit::WebView *webView;
    WebKit::WebSize size;
    skia::PlatformCanvas skiaCanvas;
    Chromix::MixRenderLoader loader;

    typedef WTF::HashMap<WTF::String, WTF::RefPtr<WebCore::ImageData> > ImageMap;
    ImageMap imageMap;
};
    
}

#endif
