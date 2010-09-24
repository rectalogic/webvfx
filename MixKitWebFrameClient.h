#ifndef MOTIONBOX_MIXKIT_WEBFRAME_CLIENT_H_
#define MOTIONBOX_MIXKIT_WEBFRAME_CLIENT_H_

#include <public/WebFrame.h>
#include <public/WebFrameClient.h>
#include <public/WebURLError.h>


class MixKitWebFrameClient : public WebKit::WebFrameClient {
public:
    MixKitWebFrameClient() : frameLoaded(false) {};

    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFinishLoad(WebKit::WebFrame*);
    bool isFrameLoaded() { return frameLoaded; };

private:
    bool frameLoaded;
};

#endif

