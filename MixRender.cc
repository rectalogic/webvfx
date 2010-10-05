#include "MixRender.h"

#include <map>
#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURL.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRect.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSettings.h>
#include <base/singleton.h>
#include <webkit/glue/webkit_glue.h>
#include <skia/ext/bitmap_platform_device.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>

typedef std::map<WebKit::WebView*, Chromix::MixRender*> ViewRenderMap;


Chromix::MixRender::MixRender(int width, int height) :
    size(width, height),
    skiaCanvas(width, height, true),
    loader(),
    imageMap()
{
    webView = WebKit::WebView::create(&loader, NULL);

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
    webView->resize(size);

    // Register in map
    Singleton<ViewRenderMap>::get()->insert(std::make_pair(webView, this));
}

Chromix::MixRender::~MixRender() {
    // Remove from map
    Singleton<ViewRenderMap>::get()->erase(webView);
    webView->close();
}

/*static*/
Chromix::MixRender* Chromix::MixRender::fromWebView(WebKit::WebView* webView) {
    ViewRenderMap* views = Singleton<ViewRenderMap>::get();
    ViewRenderMap::iterator it = views->find(webView);
    return it == views->end() ? NULL : it->second;
}

bool Chromix::MixRender::loadURL(const std::string& url) {
    return loader.loadURL(webView, url);
}

// http://webkit.org/coding/RefPtr.html
WTF::PassRefPtr<WebCore::ImageData> Chromix::MixRender::imageDataForKey(WTF::String key, unsigned int width, unsigned int height) {
    WTF::RefPtr<WebCore::ImageData> imageData;
    // Found image for key, recreate if wrong size
    if (imageMap.contains(key)) {
        imageData = imageMap.get(key);
        if (width != imageData->width() || height != imageData->height()) {
            imageData = WebCore::ImageData::create(width, height);
            imageMap.set(key, imageData);
        }
    }
    // No image in map, create a new one
    else {
        imageData = WebCore::ImageData::create(width, height);
        imageMap.set(key, imageData);
    }
    return imageData.release(); //XXX figure out refcounting
}


const SkBitmap& Chromix::MixRender::render() {
    webView->paint(webkit_glue::ToWebCanvas(&skiaCanvas), WebKit::WebRect(0, 0, size.width, size.height));

    // Get canvas bitmap
    skia::BitmapPlatformDevice &skiaDevice = static_cast<skia::BitmapPlatformDevice&>(skiaCanvas.getTopPlatformDevice());
    return skiaDevice.accessBitmap(false);
}
