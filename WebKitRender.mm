#import <Foundation/Foundation.h>
#include <public/WebKit.h>
#include <public/WebKitClient.h>
#include <public/WebView.h>
#include <public/WebFrame.h>
#include <public/WebURL.h>
#include <public/WebURLRequest.h>
#include <public/WebSize.h>
#include <webkit/glue/webkitclient_impl.h>

using WebKit::WebView;
using WebKit::WebFrame;
using WebKit::WebSize;
using WebKit::WebURL;
using WebKit::WebURLRequest;
using WebKit::WebKitClient;
using webkit_glue::WebKitClientImpl;

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    WebKitClientImpl client;
    WebKit::initialize((WebKitClient *)&client);

    WebView *webView = WebView::create(NULL, NULL);
    webView->initializeMainFrame(NULL);
    webView->resize(WebSize(400, 300));
    WebFrame *webFrame = webView->mainFrame();
    webFrame->setCanHaveScrollbars(false);
    webFrame->loadRequest(WebURLRequest(WebURL(GURL("http://www.google.com/"))));
    webView->layout();
    //webView->paint(XXX);

    WebKit::shutdown();

    [pool drain];
    return 0;
}
