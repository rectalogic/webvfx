#include "WebFrameClientImpl.h"

#include <base/message_loop.h>

void MixKit::WebFrameClientImpl::didFailLoad(WebKit::WebFrame* frame, const WebKit::WebURLError& error) {
    //XXX need to handle
    printf("Failed to load\n");
    frameLoaded = true;
    MessageLoop::current()->Quit();
}

void MixKit::WebFrameClientImpl::didFinishLoad(WebKit::WebFrame* frame) {
    //XXX this gets called twice, first with about:blank then the real url (p frame->url().m_spec.data())
    frameLoaded = true;
    MessageLoop::current()->Quit();
}