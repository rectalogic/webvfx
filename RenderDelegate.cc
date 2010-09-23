
#include "RenderDelegate.h"

void RenderDelegate::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    printf("Failed to load\n");
    frameLoaded = true;
}

void RenderDelegate::didFinishLoad(WebKit::WebFrame* frame) {
    frameLoaded = true;
}