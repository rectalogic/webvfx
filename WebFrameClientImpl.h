#ifndef MOTIONBOX_MIXKIT_WEBFRAMECLIENTIMPL_H_
#define MOTIONBOX_MIXKIT_WEBFRAMECLIENTIMPL_H_

#include <public/WebFrame.h>
#include <public/WebFrameClient.h>
#include <public/WebURLError.h>

namespace MixKit {

class WebFrameClientImpl : public WebKit::WebFrameClient {
public:
    WebFrameClientImpl() : frameLoaded(false) {};

    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFinishLoad(WebKit::WebFrame*);
    bool isFrameLoaded() { return frameLoaded; };

private:
    bool frameLoaded;
};

}
#endif

