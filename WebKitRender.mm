#import <Foundation/Foundation.h>
#include <public/WebKit.h>
#include <public/WebKitClient.h>
#include <public/WebView.h>
#include <public/WebFrame.h>
#include <public/WebURL.h>
#include <public/WebURLRequest.h>
#include <public/WebSize.h>
#include <public/WebRect.h>
#include <webkit/glue/webkitclient_impl.h>
#include <webkit/glue/webkit_glue.h>
#include <base/at_exit.h>
#include <base/basictypes.h>
#include <base/message_loop.h>
#include <skia/ext/platform_canvas.h>

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    // These setup global state
    base::AtExitManager at_exit_manager;
    MessageLoop messageLoop; //XXX test_shell uses MessageLoopForUI, but this might be OK

    webkit_glue::WebKitClientImpl client;
    WebKit::initialize((WebKit::WebKitClient *)&client);

    WebKit::WebSize size(400, 300);
    WebKit::WebView *webView = WebKit::WebView::create(NULL, NULL);
    webView->initializeMainFrame(NULL);
    webView->resize(size);
    WebKit::WebFrame *webFrame = webView->mainFrame();
    webFrame->setCanHaveScrollbars(false);
    webFrame->loadRequest(WebKit::WebURLRequest(WebKit::WebURL(GURL("http://www.google.com/"))));
    webView->layout();
    //XXX base::MessageLoop::current()->Run();

    skia::PlatformCanvas skiaCanvas(size.width, size.height, true);
    webView->paint(webkit_glue::ToWebCanvas(&skiaCanvas), WebKit::WebRect(0, 0, size.width, size.height));

    webView->close();
    WebKit::shutdown();

    [pool drain];
    return 0;
}
