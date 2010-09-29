#import <Foundation/Foundation.h>

#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURL.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSize.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRect.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSettings.h>
#include <webkit/glue/webkit_glue.h>
#include <base/basictypes.h>
#include <base/message_loop.h>
#include <skia/ext/platform_canvas.h>
#include <skia/ext/bitmap_platform_device.h>
#include <gfx/codec/png_codec.h>

#include "MixGlobal.h"
#include "WebViewDelegate.h"

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    MixKit::MixGlobal mixKit;

    WebKit::WebSize size(400, 300);
    MixKit::WebViewDelegate delegate;
    WebKit::WebView *webView = WebKit::WebView::create(&delegate, NULL);
    webView->initializeMainFrame(&delegate);
    webView->resize(size);
    WebKit::WebFrame *webFrame = webView->mainFrame();
    webFrame->setCanHaveScrollbars(false);
    webView->settings()->setLoadsImagesAutomatically(true);
    webFrame->loadRequest(WebKit::WebURLRequest(WebKit::WebURL(GURL("test.html"))));
    webView->layout();
    while (!delegate.isLoadFinished())
        MessageLoop::current()->Run();

    //XXX Probably need to do this periodically when rendering video frames
    webFrame->collectGarbage();

    skia::PlatformCanvas skiaCanvas(size.width, size.height, true);
    webView->paint(webkit_glue::ToWebCanvas(&skiaCanvas), WebKit::WebRect(0, 0, size.width, size.height));

    {
        // Get canvas bitmap
        skia::BitmapPlatformDevice &skiaDevice = static_cast<skia::BitmapPlatformDevice&>(skiaCanvas.getTopPlatformDevice());
        const SkBitmap &skiaBitmap = skiaDevice.accessBitmap(false);

        // Encode pixel data to PNG.
        std::vector<unsigned char> pngData;
        SkAutoLockPixels bitmapLock(skiaBitmap);
        gfx::PNGCodec::Encode(reinterpret_cast<const unsigned char*>(skiaBitmap.getPixels()),
                              gfx::PNGCodec::FORMAT_BGRA, skiaBitmap.width(), skiaBitmap.height(),
                              static_cast<int>(skiaBitmap.rowBytes()), false, &pngData);

        // Write to disk. Cheat and be MacOS specific since this is just for testing.
        NSData *data = [NSData dataWithBytesNoCopy:reinterpret_cast<void *>(&pngData[0])
                                            length:pngData.size()
                                      freeWhenDone:NO];
        [data writeToFile:@"/tmp/render.png" atomically:NO];
    }

    // Shutdown
    webView->close();

    [pool drain];
    return 0;
}
