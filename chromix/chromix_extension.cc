// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/chromix_extension.h"
#include "chromix/scripting_support.h"

#include <v8/include/v8.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>

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
                      "chromix.getParamValue = function(name) {"
                      "    native function GetParamValue();"
                      "    return GetParamValue(name);"
                      "};"
                      "chromix.getImageParamValue = function(name) {"
                      "    native function GetImageParamValue();"
                      "    return GetImageParamValue(name);"
                      "};"
                      ) {}

    virtual v8::Handle<v8::FunctionTemplate> GetNativeFunction(v8::Handle<v8::String> name) {
        if (name->Equals(v8::String::New("GetParamValue"))) {
            return v8::FunctionTemplate::New(GetParamValue);
        }
        else if (name->Equals(v8::String::New("GetImageParamValue"))) {
            return v8::FunctionTemplate::New(GetImageParamValue);
        }
        else if (name->Equals(v8::String::New("SetRenderCallback"))) {
            return v8::FunctionTemplate::New(SetRenderCallback);
        }

        return v8::Handle<v8::FunctionTemplate>();
    }

    static v8::Handle<v8::Value> GetParamValue(const v8::Arguments& args) {
        if (args.Length() >= 1 && args[0]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                return scriptingSupport->getParameterValue(*v8::String::Utf8Value(args[0]));
        }
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> GetImageParamValue(const v8::Arguments& args) {
        if (args.Length() >= 1 && args[0]->IsString()) {
            Chromix::ScriptingSupport *scriptingSupport = findScriptingSupport();
            if (scriptingSupport)
                return scriptingSupport->getImageParameterValue(*v8::String::Utf8Value(args[0]));
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
