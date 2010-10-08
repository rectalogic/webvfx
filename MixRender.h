#ifndef MOTIONBOX_CHROMIX_MIXRENDER_H_
#define MOTIONBOX_CHROMIX_MIXRENDER_H_

#include "Loader.h"

#include <string>
#include <skia/ext/platform_canvas.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSize.h>

namespace WTF {
    class String;
}

namespace Chromix {

class ScriptingSupport;

class MixRender {
public:
    MixRender();
    virtual ~MixRender();

    bool loadURL(const std::string& url);
    void resize(int width, int height);
    const SkBitmap& render(double time);

    unsigned char* writeableDataForImageParameter(const WTF::String& name, unsigned int width, unsigned int height);

    //XXX make these protected and friend to ChromixExtension?
    static ScriptingSupport* scriptingSupportFromWebView(WebKit::WebView* webView);

private:
    WebKit::WebView *webView;
    WebKit::WebSize size;
    skia::PlatformCanvas* skiaCanvas;
    Loader loader;
    ScriptingSupport *scriptingSupport;
};
    
}

#endif
