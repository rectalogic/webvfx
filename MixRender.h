#ifndef MOTIONBOX_CHROMIX_MIXRENDER_H_
#define MOTIONBOX_CHROMIX_MIXRENDER_H_

#include "MixRenderLoader.h"

#include <third_party/WebKit/WebKit/chromium/public/WebSize.h>
#include <skia/ext/platform_canvas.h>


namespace Chromix {

class MixParameterMap;

class MixRender {
public:
    //XXX need to allow caller to resize on the fly before rendering (need to resize webview, skia canvas etc.)
    MixRender(int width, int height);
    virtual ~MixRender();

    bool loadURL(const std::string& url);
    const SkBitmap& render(float time);

    MixParameterMap* getParameterMap() { return parameterMap; }

    static MixRender* fromWebView(WebKit::WebView* webView);

    //XXX need setParameters() api called from v8, and param/imageForKey called from v8

private:
    WebKit::WebView *webView;
    WebKit::WebSize size;
    skia::PlatformCanvas skiaCanvas;
    MixRenderLoader loader;
    MixParameterMap *parameterMap;
};
    
}

#endif
