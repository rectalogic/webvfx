#include "MixRender.h"

#include <third_party/WebKit/WebKit/chromium/public/WebURL.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRect.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSettings.h>
#include <base/message_loop.h>
#include <webkit/glue/webkit_glue.h>
#include <skia/ext/bitmap_platform_device.h>

Chromix::MixRender::MixRender(int width, int height) :
    size(width, height),
    skiaCanvas(width, height, true),
    inMessageLoop(false),
    isLoadFinished(false)
{
    webView = WebKit::WebView::create(this, NULL);
    webView->initializeMainFrame(this);
    webView->resize(size);
    //webView->mainFrame()->setCanHaveScrollbars(false);
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
    settings->setJavaEnabled(false);
    settings->setPluginsEnabled(true);
    settings->setDOMPasteAllowed(false);
    settings->setUsesPageCache(false);
    settings->setJavaScriptCanOpenWindowsAutomatically(false);
    settings->setJavaScriptEnabled(true);
    settings->setLoadsImagesAutomatically(true);
    settings->setAuthorAndUserStylesEnabled(true);
    //settings->setAcceleratedCompositingEnabled(true);
    //settings->setAccelerated2dCanvasEnabled(true);
    settings->setShouldPaintCustomScrollbars(false);
}

Chromix::MixRender::~MixRender() {
    webView->close();
}

bool Chromix::MixRender::loadURL(const std::string& url) {
    WebKit::WebFrame *webFrame = webView->mainFrame();
    webFrame->loadRequest(WebKit::WebURLRequest(WebKit::WebURL(GURL(url))));
    webView->layout();//XXX need this?
    while (!isLoadFinished) {
        inMessageLoop = true;
        MessageLoop::current()->Run();
        inMessageLoop = false;
    }
    
    //XXX Probably need to do this periodically when rendering video frames
    //XXXcrashes webFrame->collectGarbage();

    return true;
}

const SkBitmap& Chromix::MixRender::render() {
    webView->paint(webkit_glue::ToWebCanvas(&skiaCanvas), WebKit::WebRect(0, 0, size.width, size.height));

    // Get canvas bitmap
    skia::BitmapPlatformDevice &skiaDevice = static_cast<skia::BitmapPlatformDevice&>(skiaCanvas.getTopPlatformDevice());
    return skiaDevice.accessBitmap(false);
}

void Chromix::MixRender::didStopLoading() {
    isLoadFinished = true;
    if (inMessageLoop)
        MessageLoop::current()->Quit();
}

void Chromix::MixRender::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    //XXX need to handle - this means one of the frames failed - set flag so we can return from loadURL
    printf("Failed to load\n");
    isLoadFinished = true;
    if (inMessageLoop)
        MessageLoop::current()->Quit();
}
