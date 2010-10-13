#ifndef MOTIONBOX_CHROMIX_SCRIPTINGSUPPORT_H_
#define MOTIONBOX_CHROMIX_SCRIPTINGSUPPORT_H_

#include "ParameterMap.h"

#include <v8/include/v8.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptState.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptValue.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptFunctionCall.h>

namespace Chromix {

class ScriptingSupport {
public:
    ScriptingSupport();
    virtual ~ScriptingSupport();

    void initialize(WebKit::WebFrame* webFrame);

    ParameterMap& getParameterMap() { return parameterMap; }

    void setRenderCallback(v8::Handle<v8::Value> callbackFunction);
    bool hasRenderCallback() { return renderCallback.hasNoValue(); }
    bool invokeRenderCallback(double time);

private:
    ParameterMap parameterMap;
    WebCore::ScriptState *scriptState;
    WebCore::ScriptValue renderCallback;
};

}

#endif
