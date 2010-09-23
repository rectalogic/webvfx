#import <Foundation/Foundation.h>
#include <public/WebKit.h>
#include <public/WebKitClient.h>
#include <public/WebView.h>
#include <public/WebFrame.h>
#include <public/WebURL.h>
#include <public/WebURLRequest.h>
#include <public/WebSize.h>
#include <webkit/glue/webkitclient_impl.h>
#include <base/at_exit.h>
#include <base/basictypes.h>
#include <base/message_loop.h>

using WebKit::WebView;
using WebKit::WebFrame;
using WebKit::WebSize;
using WebKit::WebURL;
using WebKit::WebURLRequest;
using WebKit::WebKitClient;
using webkit_glue::WebKitClientImpl;

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    // These setup global state
    base::AtExitManager at_exit_manager;
    MessageLoop messageLoop; //XXX test_shell uses MessageLoopForUI, but this might be OK

    WebKitClientImpl client;
    WebKit::initialize((WebKitClient *)&client);

    WebView *webView = WebView::create(NULL, NULL);
    webView->initializeMainFrame(NULL);
    webView->resize(WebSize(400, 300));
    WebFrame *webFrame = webView->mainFrame();
    webFrame->setCanHaveScrollbars(false);
    webFrame->loadRequest(WebURLRequest(WebURL(GURL("http://www.google.com/"))));
    webView->layout();
    //XXX base::MessageLoop::current()->Run();
    //webView->paint(XXX);

    webView->close();
    WebKit::shutdown();

    [pool drain];
    return 0;
}
