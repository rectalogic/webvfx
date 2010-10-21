#ifndef MOTIONBOX_CHROMIX_PARAMETERMAP_H_
#define MOTIONBOX_CHROMIX_PARAMETERMAP_H_

#include "chromix/ParameterValue.h"

#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefPtr.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/StringHash.h>
#include <third_party/WebKit/JavaScriptCore/wtf/HashMap.h>

namespace v8 {
    class Value;
    template <class T> class Handle;
}

namespace Chromix {

class MixRender;

class ParameterMap {
public:
    ParameterMap();
    virtual ~ParameterMap() {};

    void setParameterValue(const WTF::String& name, WTF::PassRefPtr<ParameterValue> prpParam);

    // Returns buffer to write image data to - RGBA format, so size is width*height*4
    unsigned char* writeableDataForImageParameter(const WTF::String& name, unsigned int width, unsigned int height);

    v8::Handle<v8::Value> getImageParameterValue(const WTF::String& name) const;
    v8::Handle<v8::Value> getParameterValue(const WTF::String& name) const;

    //XXX APIs to enumerate (keys and descriptions)

private:
    typedef WTF::HashMap<WTF::String, ImageParameterValue> ImageParamMap;
    ImageParamMap imageParamMap;

    typedef WTF::HashMap<WTF::String, WTF::RefPtr<ParameterValue> > ParamMap;
    ParamMap paramMap;
};

}

#endif
