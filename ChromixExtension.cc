#include "ChromixExtension.h"
#include "MixRender.h"
#include "ScriptingSupport.h"
#include "ParameterMap.h"

#include <v8/include/v8.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
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
                      "chromix.registerImageParam = function(name, description) {"
                      "    native function RegisterImageParam();"
                      "    RegisterImageParam(name, description);"
                      "};"
                      "chromix.getStringParamValue = function(name) {"
                      "    native function GetStringParamValue();"
                      "    return GetStringParamValue(name);"
                      "};"
                      "chromix.getImageParamValue = function(name) {"
                      "    native function GetImageParamValue();"
                      "    return GetImageParamValue(name);"
                      "};"
                      ) {}
    //XXX add chromix.log() method so user can manage error output?

    //XXX getting image/time based params should only be valid when responding to RenderEvent/callback (set flag and check we are rendering)
    //XXX static params should be gotten up front right after registering them??
    //XXX or series of chromix.registerStringParam(name, desc) and chromix.registerImageDataParam(name, desc) and single chromix.registerMix(name, renderCallback)
    //XXX and chromix.getStringParam(name) chromix.getImageDataParam(name)
    //XXX std::string name = std::string(*v8::String::Utf8Value(args[0])), also see v8StringToWebCoreString

    virtual v8::Handle<v8::FunctionTemplate> GetNativeFunction(v8::Handle<v8::String> name) {
        if (name->Equals(v8::String::New("RegisterStringParam"))) {
            return v8::FunctionTemplate::New(RegisterStringParam);
        }
        else if (name->Equals(v8::String::New("RegisterImageParam"))) {
            return v8::FunctionTemplate::New(RegisterImageParam);
        }
        else if (name->Equals(v8::String::New("GetStringParamValue"))) {
            return v8::FunctionTemplate::New(GetStringParamValue);
        }
        else if (name->Equals(v8::String::New("GetImageParamValue"))) {
            return v8::FunctionTemplate::New(GetImageParamValue);
        }

        return v8::Handle<v8::FunctionTemplate>();
    }

    static v8::Handle<v8::Value> RegisterStringParam(const v8::Arguments& args) {
        if (args.Length() >= 2 && args[0]->IsString() && args[1]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                scriptingSupport->getParameterMap().registerStringParameter(*v8::String::Utf8Value(args[0]), *v8::String::Utf8Value(args[1]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> RegisterImageParam(const v8::Arguments& args) {
        if (args.Length() >= 2 && args[0]->IsString() && args[1]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                scriptingSupport->getParameterMap().registerImageParameter(*v8::String::Utf8Value(args[0]), *v8::String::Utf8Value(args[1]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> GetStringParamValue(const v8::Arguments& args) {
        if (args.Length() >= 1 && args[0]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                return scriptingSupport->getParameterMap().getStringParameterValue(*v8::String::Utf8Value(args[0]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> GetImageParamValue(const v8::Arguments& args) {
        if (args.Length() >= 1 && args[0]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                return scriptingSupport->getParameterMap().getImageParameterValue(*v8::String::Utf8Value(args[0]));
        }
        return v8::Undefined();
    }

private:
    static Chromix::ScriptingSupport* findScriptingSupport() {
        return Chromix::MixRender::scriptingSupportFromWebView(WebKit::WebFrame::frameForEnteredContext()->view());
    }
};

v8::Extension* Chromix::ChromixExtension::Get() {
    return new ChromixExtensionWrapper();
}

}
