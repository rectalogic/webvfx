#ifndef MOTIONBOX_CHROMIX_MIXPARAMETERMAP_H_
#define MOTIONBOX_CHROMIX_MIXPARAMETERMAP_H_

#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefPtr.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/StringHash.h>
#include <third_party/WebKit/JavaScriptCore/wtf/HashMap.h>


namespace Chromix {

class MixParameterMap {
public:
    MixParameterMap();
    virtual ~MixParameterMap() {};
    
    WTF::PassRefPtr<WebCore::ImageData> imageDataForKey(WTF::String key, unsigned int width, unsigned int height);

private:
    typedef WTF::HashMap<WTF::String, WTF::RefPtr<WebCore::ImageData> > ImageParamMap;
    ImageParamMap imageMap;
};

}

#endif
