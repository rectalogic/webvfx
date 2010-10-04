#include "MixRender.h"

#include <third_party/WebKit/WebKit/chromium/public/WebURL.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRect.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSettings.h>
#include <base/logging.h>
#include <base/message_loop.h>
#include <webkit/glue/webkit_glue.h>
#include <skia/ext/bitmap_platform_device.h>


Chromix::MixRender::MixRender(int width, int height) :
    size(width, height),
    skiaCanvas(width, height, true),
    inMessageLoop(false),
    isLoadFinished(false),
    didLoadSucceed(false)
{
    webView = WebKit::WebView::create(this, NULL);

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

    webView->initializeMainFrame(this);
    //webView->mainFrame()->setCanHaveScrollbars(false);
    webView->resize(size);
}

Chromix::MixRender::~MixRender() {
    webView->close();
}

bool Chromix::MixRender::loadURL(const std::string& url) {
    isLoadFinished = false;
    didLoadSucceed = false;

    WebKit::WebFrame *webFrame = webView->mainFrame();
    GURL gurl(url);
    if (!gurl.is_valid())
        return didLoadSucceed;
    webFrame->loadRequest(WebKit::WebURLRequest(gurl));
    webView->layout();//XXX need this?
    while (!isLoadFinished) {
        inMessageLoop = true;
        MessageLoop::current()->Run();
        inMessageLoop = false;
    }
    
    //XXX Probably need to do this periodically when rendering video frames
    //XXXcrashes webFrame->collectGarbage();

    return didLoadSucceed;
}

const SkBitmap& Chromix::MixRender::render() {
    DLOG_ASSERT(didLoadSucceed);
    webView->paint(webkit_glue::ToWebCanvas(&skiaCanvas), WebKit::WebRect(0, 0, size.width, size.height));

    // Get canvas bitmap
    skia::BitmapPlatformDevice &skiaDevice = static_cast<skia::BitmapPlatformDevice&>(skiaCanvas.getTopPlatformDevice());
    return skiaDevice.accessBitmap(false);
}

void Chromix::MixRender::didStopLoading() {
    // This is called even after load failure, so don't reset flags if we already failed.
    if (!isLoadFinished) {
        isLoadFinished = true;
        didLoadSucceed = true;
    }
    if (inMessageLoop)
        MessageLoop::current()->Quit();
}

void Chromix::MixRender::didFailProvisionalLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    handlLoadFailure(error);
}

void Chromix::MixRender::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    handlLoadFailure(error);
}

void Chromix::MixRender::handlLoadFailure(const WebKit::WebURLError& error) {
    printf("Load failed %d\n", error.reason);
    isLoadFinished = true;
    didLoadSucceed = false;
    if (inMessageLoop)
        MessageLoop::current()->Quit();
}
