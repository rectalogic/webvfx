#include "ImageExtension.h"
#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>
#include <V8ImageData.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefCounted.h>

namespace Chromix {

const char *kImageExtensionName = "ChromixV8/Image";

//XXX should we fire an event via the DOM for each time signal (e.g. dispatch via WebFrame::windowObject)?
//XXX will need a way to get the set of named currentFrame images into this extension
class ImageWrapper : public v8::Extension {
public:
    ImageWrapper() :
        v8::Extension(kImageExtensionName,
                      "if (typeof(chromix) == 'undefined') {"
                      "    chromix = {};"
                      "};"
                      "chromix.getImageDataForKey = function(key) {"
                      "    native function GetImageDataForKey();"
                      "    return GetImageDataForKey(key);"
                      "};"
                      ) {}

    virtual v8::Handle<v8::FunctionTemplate> GetNativeFunction(v8::Handle<v8::String> name) {
        if (name->Equals(v8::String::New("GetImageDataForKey"))) {
            return v8::FunctionTemplate::New(GetImageDataForKey);
         }
        
        return v8::Handle<v8::FunctionTemplate>();
    }

    //XXX how do we get at instance data? image key hash will need to be global registry
    //XXX we don't want to copy pixels from decode buffer into ImageData CanvasPixelArray - so we should preconstruct JS ImageData objects for each key (at correct size), and decode directly into their raw buffers
    //XXX see chrome/renderer/extensions/extension_process_bindings.cc - uses static maps - but we need keys to be scoped to a page
    //XXX also chrome/renderer/external_extension.cc - WebFrame::frameForEnteredContext()->view()
    
    //XXX maybe JS should create ImageDatas of view size and call back to fill???

    static v8::Handle<v8::Value> GetImageDataForKey(const v8::Arguments& args) {
        
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

v8::Extension* Chromix::ImageExtensionV8::Get() {
    return new ImageWrapper();
}

}
