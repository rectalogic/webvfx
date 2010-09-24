#include "WebViewDelegate.h"

#include <base/message_loop.h>



void MixKit::WebViewDelegate::didStopLoading() {
    loadFinished = true;
    MessageLoop::current()->Quit();
}

void MixKit::WebViewDelegate::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    //XXX need to handle - this means one of the frames failed
    printf("Failed to load\n");
    loadFinished = true;
    MessageLoop::current()->Quit();
}
