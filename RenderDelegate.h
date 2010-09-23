#ifndef MOTIONBOX_RENDER_DELEGATE_H_
#define MOTIONBOX_RENDER_DELEGATE_H_

#include <public/WebFrame.h>
#include <public/WebFrameClient.h>
#include <public/WebURLError.h>

class RenderDelegate : public WebKit::WebFrameClient {
public:
    RenderDelegate() : frameLoaded(false) {};

    virtual void didFailLoad(WebKit::WebFrame*, const WebKit::WebURLError&);
    virtual void didFinishLoad(WebKit::WebFrame*);
    bool isFrameLoaded() { return frameLoaded; };
private:
    bool frameLoaded;
};

#endif

