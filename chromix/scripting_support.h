// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_SCRIPTING_SUPPORT_H_
#define CHROMIX_SCRIPTING_SUPPORT_H_

#include "chromix/image_map.h"

#include <base/basictypes.h>
#include <v8/include/v8.h>
#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptState.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptValue.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptFunctionCall.h>

namespace Chromix {

class Delegate;

class ScriptingSupport {
public:
    ScriptingSupport(Delegate* delegate);
    virtual ~ScriptingSupport();

    static ScriptingSupport* fromWebView(WebKit::WebView* webView);

    // webView must have its mainFrame created
    void initialize(WebKit::WebView* webView);

    v8::Handle<v8::Value> getImageParameterValue(const std::string& name) { return imageMap.getImageParameterValue(name); }
    v8::Handle<v8::Value> getParameterValue(const std::string& name);
    unsigned char* writeableDataForImageParameter(const std::string& name, unsigned int width, unsigned int height) { return imageMap.writeableDataForImageParameter(name, width, height); }

    void setRenderCallback(v8::Handle<v8::Value> callbackFunction);
    bool hasRenderCallback() { return !renderCallback.hasNoValue(); }
    bool invokeRenderCallback(double time);

private:
    Delegate* delegate;
    WebKit::WebView *webView;
    ImageMap imageMap;
    WebCore::ScriptState *scriptState;
    WebCore::ScriptValue renderCallback;

    DISALLOW_COPY_AND_ASSIGN(ScriptingSupport);
};

}

#endif
