#ifndef CHROMIX_SCRIPTINGSUPPORT_H_
#define CHROMIX_SCRIPTINGSUPPORT_H_

#include "chromix/ParameterMap.h"

#include <base/basictypes.h>
#include <v8/include/v8.h>
#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptState.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptValue.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptFunctionCall.h>

namespace Chromix {

class ScriptingSupport {
public:
    ScriptingSupport();
    virtual ~ScriptingSupport();

    static ScriptingSupport* fromWebView(WebKit::WebView* webView);

    // webView must have its mainFrame created
    void initialize(WebKit::WebView* webView);

    ParameterMap& getParameterMap() { return parameterMap; }

    void setRenderCallback(v8::Handle<v8::Value> callbackFunction);
    bool hasRenderCallback() { return !renderCallback.hasNoValue(); }
    bool invokeRenderCallback(double time);

private:
    WebKit::WebView *webView;
    ParameterMap parameterMap;
    WebCore::ScriptState *scriptState;
    WebCore::ScriptValue renderCallback;

    DISALLOW_COPY_AND_ASSIGN(ScriptingSupport);
};

}

#endif
