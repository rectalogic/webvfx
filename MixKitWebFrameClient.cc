#include "MixKitWebFrameClient.h"

#include <base/message_loop.h>

void MixKitWebFrameClient::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    //XXX need to handle
    printf("Failed to load\n");
    frameLoaded = true;
    MessageLoop::current()->Quit();
}

void MixKitWebFrameClient::didFinishLoad(WebKit::WebFrame* frame) {
    frameLoaded = true;
    MessageLoop::current()->Quit();
}