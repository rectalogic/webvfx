#include "MixRender.h"
#include "ScriptingSupport.h"
#include "ParameterMap.h"

#include <map>
#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURL.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRect.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSettings.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <base/singleton.h>
#include <webkit/glue/webkit_glue.h>
#include <skia/ext/bitmap_platform_device.h>

typedef std::map<WebKit::WebView*, Chromix::ScriptingSupport*> ViewScriptMap;

Chromix::MixRender::MixRender() :
    size(),
    skiaCanvas(NULL),
    loader(),
    scriptingSupport(new Chromix::ScriptingSupport())
{
    webView = WebKit::WebView::create(&loader, NULL);

    // Register in map
    Singleton<ViewScriptMap>::get()->insert(std::make_pair(webView, scriptingSupport));

    WebKit::WebSettings *settings = webView->settings();
    settings->setStandardFontFamily(WebKit::WebString("Times New Roman"));
    settings->setFixedFontFamily(WebKit::WebString("Courier New"));
    settings->setSerifFontFamily(WebKit::WebString("Times New Roman"));
    settings->setSansSerifFontFamily(WebKit::WebString("Arial"));
    settings->setCursiveFontFamily(WebKit::WebString("Comic Sans MS"));
    settings->setFantasyFontFamily(WebKit::WebString("Times New Roman"));
    settings->setDefaultFontSize(16);
    settings->setDefaultFixedFontSize(13);
    settings->setMinimumFontSize(1);
    settings->setJavaScriptEnabled(true);
    settings->setJavaScriptCanOpenWindowsAutomatically(false);
    settings->setLoadsImagesAutomatically(true);
    settings->setImagesEnabled(true);
    settings->setPluginsEnabled(true);
    settings->setDOMPasteAllowed(false);
    settings->setJavaEnabled(false);
    settings->setJavaEnabled(false);
    settings->setAllowScriptsToCloseWindows(false);
    settings->setUsesPageCache(false);
    settings->setShouldPaintCustomScrollbars(false);
    settings->setAllowUniversalAccessFromFileURLs(true);
    settings->setAllowFileAccessFromFileURLs(true);
    settings->setExperimentalWebGLEnabled(true);
    settings->setAcceleratedCompositingEnabled(false);//XXX crashes WebGL
    settings->setAccelerated2dCanvasEnabled(true);

    webView->initializeMainFrame(&loader);
    //webView->mainFrame()->setCanHaveScrollbars(false);
}

Chromix::MixRender::~MixRender() {
    // Remove from map
    Singleton<ViewScriptMap>::get()->erase(webView);
    delete skiaCanvas;
    delete scriptingSupport;
    webView->close();
}

/*static*/
Chromix::ScriptingSupport* Chromix::MixRender::scriptingSupportFromWebView(WebKit::WebView* webView) {
    ViewScriptMap* views = Singleton<ViewScriptMap>::get();
    ViewScriptMap::iterator it = views->find(webView);
    return it == views->end() ? NULL : it->second;
}

bool Chromix::MixRender::loadURL(const std::string& url) {
    return loader.loadURL(webView, url);
}

void Chromix::MixRender::resize(int width, int height) {
    if (size.width == width && size.height == height)
        return;
    size.width = width;
    size.height = height;
    webView->resize(size);
    // NULL out so we recreate canvas in render()
    delete skiaCanvas;
    skiaCanvas = NULL;
}

const SkBitmap* Chromix::MixRender::render(double time) {
    if (skiaCanvas == NULL)
        skiaCanvas = new skia::PlatformCanvas(size.width, size.height, true);

    //XXX set flag allowing image param access (so only allowed while rendering)
    if (!scriptingSupport->invokeRenderCallback(time))
        return NULL;

    webView->layout();
    webView->paint(webkit_glue::ToWebCanvas(skiaCanvas), WebKit::WebRect(0, 0, size.width, size.height));

    // Get canvas bitmap
    skia::BitmapPlatformDevice &skiaDevice = static_cast<skia::BitmapPlatformDevice&>(skiaCanvas->getTopPlatformDevice());
    return &skiaDevice.accessBitmap(false);
}

unsigned char* Chromix::MixRender::writeableDataForImageParameter(const WTF::String& name, unsigned int width, unsigned int height) {
    return scriptingSupport->getParameterMap().writeableDataForImageParameter(name, width, height);
}

