#include "chromix/ChromixExtension.h"
#include "chromix/ScriptingSupport.h"
#include "chromix/ParameterMap.h"

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
                      "    native function SetRenderCallback();"
                      "    SetRenderCallback(renderCallback);"
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
        else if (name->Equals(v8::String::New("SetRenderCallback"))) {
            return v8::FunctionTemplate::New(SetRenderCallback);
        }

        return v8::Handle<v8::FunctionTemplate>();
    }

    static v8::Handle<v8::Value> RegisterStringParam(const v8::Arguments& args) {
        if (args.Length() >= 2 && args[0]->IsString() && args[1]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                scriptingSupport->getParameterMap().registerStringParameter(*v8::String::Value(args[0]), *v8::String::Value(args[1]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> RegisterImageParam(const v8::Arguments& args) {
        if (args.Length() >= 2 && args[0]->IsString() && args[1]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                scriptingSupport->getParameterMap().registerImageParameter(*v8::String::Value(args[0]), *v8::String::Value(args[1]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> GetStringParamValue(const v8::Arguments& args) {
        if (args.Length() >= 1 && args[0]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                return scriptingSupport->getParameterMap().getStringParameterValue(*v8::String::Value(args[0]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> GetImageParamValue(const v8::Arguments& args) {
        if (args.Length() >= 1 && args[0]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                return scriptingSupport->getParameterMap().getImageParameterValue(*v8::String::Value(args[0]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> SetRenderCallback(const v8::Arguments& args) {
        if (args.Length() >= 1 && args[0]->IsFunction()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                scriptingSupport->setRenderCallback(args[0]);
        }
        return v8::Undefined();
    }

private:
    static Chromix::ScriptingSupport* findScriptingSupport() {
        return Chromix::ScriptingSupport::fromWebView(WebKit::WebFrame::frameForEnteredContext()->view());
    }
};

v8::Extension* Chromix::ChromixExtension::Get() {
    return new ChromixExtensionWrapper();
}

}
