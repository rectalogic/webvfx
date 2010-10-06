#include "ChromixExtension.h"

#include <v8/include/v8.h>
#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefCounted.h>
#include <V8ImageData.h>

namespace Chromix {

const char *kChromixExtensionName = "v8Chromix/Chromix";

class ChromixExtensionWrapper : public v8::Extension {
public:
    ChromixExtensionWrapper() :
        v8::Extension(kChromixExtensionName,
                      "if (typeof(chromix) == 'undefined') {"
                      "    chromix = {};"
                      "};"
                      "chromix.setRenderCallback = function(renderCallback) {"
                      "    chromix.renderCallback = renderCallback;"
                      "};"
                      "chromix.registerStringParam = function(name, description) {"
                      "    native function RegisterStringParam();"
                      "    RegisterStringParam(name, description);"
                      "};"
                      "chromix.registerImageDataParam = function(name, description) {"
                      "    native function RegisterImageDataParam();"
                      "    RegisterImageDataParam(name, description);"
                      "};"
                      "chromix.getStringParamValue = function(name) {"
                      "    native function GetStringParamValue();"
                      "    return GetStringParamValue(name);"
                      "};"
                      "chromix.getImageDataParamValue = function(name) {"
                      "    native function GetImageDataParamValue();"
                      "    return GetImageDataParamValue(name);"
                      "};"
                      ) {}
    //XXX getting image/time based params should only be valid when responding to RenderEvent/callback (set flag and check we are rendering)
    //XXX static params should be gotten up front right after registering them??
    //XXX or series of chromix.registerStringParam(name, desc) and chromix.registerImageDataParam(name, desc) and single chromix.registerMix(name, renderCallback)
    //XXX and chromix.getStringParam(name) chromix.getImageDataParam(name)
    //XXX std::string name = std::string(*v8::String::Utf8Value(args[0])), also see v8StringToWebCoreString

    virtual v8::Handle<v8::FunctionTemplate> GetNativeFunction(v8::Handle<v8::String> name) {
        if (name->Equals(v8::String::New("RegisterStringParam"))) {
            return v8::FunctionTemplate::New(RegisterStringParam);
        }
        else if (name->Equals(v8::String::New("RegisterImageDataParam"))) {
            return v8::FunctionTemplate::New(RegisterImageDataParam);
        }
        else if (name->Equals(v8::String::New("GetStringParamValue"))) {
            return v8::FunctionTemplate::New(GetStringParamValue);
        }
        else if (name->Equals(v8::String::New("GetImageDataParamValue"))) {
            return v8::FunctionTemplate::New(GetImageDataParamValue);
        }

        return v8::Handle<v8::FunctionTemplate>();
    }

    //XXX how do we get at instance data? image key hash will need to be global registry
    //XXX we don't want to copy pixels from decode buffer into ImageData CanvasPixelArray - so we should preconstruct JS ImageData objects for each key (at correct size), and decode directly into their raw buffers
    //XXX see chrome/renderer/extensions/extension_process_bindings.cc - uses static maps - but we need keys to be scoped to a page
    //XXX also chrome/renderer/external_extension.cc - WebFrame::frameForEnteredContext()->view()
    
    //XXX maybe JS should create ImageDatas of view size and call back to fill???

    static v8::Handle<v8::Value> RegisterStringParam(const v8::Arguments& args) {
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> RegisterImageDataParam(const v8::Arguments& args) {
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> GetStringParamValue(const v8::Arguments& args) {
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> GetImageDataParamValue(const v8::Arguments& args) {
        
        //XXX see third_party/WebKit/WebCore/bindings/v8/SerializedScriptValue.cpp
        WTF::PassRefPtr<WebCore::ImageData> imageData = WebCore::ImageData::create(320, 240);//XXX bogus size for now
        WTF::ByteArray* pixelArray = imageData->data()->data();

        //XXX write to pixelArray->data()
        for (unsigned int i = 0; i < pixelArray->length(); i += 4) {
            pixelArray->set(i, (unsigned char)0xff); // red
            pixelArray->set(i+3, (unsigned char)0xff); // alpha
        }

        //XXX this will RefCounted<ImageData>::ref() imageData, above ::create does PassRefPtr<ImageData>::adoptRef()
        //XXX so we need to release the imageData when all done (i.e. won't be GC'd) ?
        return WebCore::toV8(imageData);
    }
};

v8::Extension* Chromix::ChromixExtension::Get() {
    return new ChromixExtensionWrapper();
}

}
