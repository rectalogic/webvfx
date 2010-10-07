#ifndef MOTIONBOX_CHROMIX_PARAMETERMAP_H_
#define MOTIONBOX_CHROMIX_PARAMETERMAP_H_

#include "Parameter.h"

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

    void registerStringParameter(const WTF::String& name, const WTF::String& description);
    void registerImageParameter(const WTF::String& name, const WTF::String& description);

    // Returns buffer to write image data to - RGBA format, so size is width*height*4
    unsigned char* writeableDataForImageParameter(const WTF::String& name, unsigned int width, unsigned int height);

    v8::Handle<v8::Value> getImageParameterValue(const WTF::String& name);
    v8::Handle<v8::Value> getStringParameterValue(const WTF::String& name);

    //XXX APIs to enumerate (keys and descriptions)

private:
    typedef WTF::HashMap<WTF::String, ImageParameter> ImageParamMap;
    ImageParamMap imageParamMap;

    typedef WTF::HashMap<WTF::String, StringParameter> StringParamMap;
    StringParamMap stringParamMap;
};

}

#endif
