#include "MixKitWebFrameClient.h"

void MixKitWebFrameClient::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    //XXX need to handle
    printf("Failed to load\n");
    frameLoaded = true;
}

void MixKitWebFrameClient::didFinishLoad(WebKit::WebFrame* frame) {
    frameLoaded = true;
}