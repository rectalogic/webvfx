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

    void setParameterValue(WTF::String const& name, ParameterValue const &param);

    // Returns buffer to write image data to - RGBA format, so size is width*height*4
    unsigned char* writeableDataForImageParameter(WTF::String const& name, unsigned int width, unsigned int height);

    v8::Handle<v8::Value> getImageParameterValue(WTF::String const& name) const;
    v8::Handle<v8::Value> getParameterValue(WTF::String const& name) const;

    //XXX APIs to enumerate (keys and descriptions)

private:
    typedef WTF::HashMap<WTF::String, ImageParameterValue> ImageParamMap;
    ImageParamMap imageParamMap;

    //XXX this is broken, can't store polymorphic by value
    typedef WTF::HashMap<WTF::String, ParameterValue> ParamMap;
    ParamMap paramMap;
};

}

#endif
