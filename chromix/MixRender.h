#ifndef MOTIONBOX_CHROMIX_MIXRENDER_H_
#define MOTIONBOX_CHROMIX_MIXRENDER_H_

#include "chromix/Loader.h"
#include "chromix/ParameterValue.h"

#include <base/basictypes.h>
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

    void setLogger(LogCallback logger, const void* data = NULL) { loader.setLogger(logger, data); }

    bool loadURL(WTF::String const& url);
    void resize(int width, int height);
    const SkBitmap* render(double time);

    void setParameterValue(WTF::String const& name, WTF::PassRefPtr<ParameterValue> value);
    unsigned char* writeableDataForImageParameter(WTF::String const& name, unsigned int width, unsigned int height);

private:
    WebKit::WebView *webView;
    WebKit::WebSize size;
    skia::PlatformCanvas* skiaCanvas;
    Loader loader;
    ScriptingSupport *scriptingSupport;

    DISALLOW_COPY_AND_ASSIGN(MixRender);
};
    
}

#endif
