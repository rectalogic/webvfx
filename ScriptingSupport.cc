#include "ScriptingSupport.h"


#include <third_party/WebKit/WebCore/bindings/v8/ScriptScope.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptFunctionCall.h>

Chromix::ScriptingSupport::ScriptingSupport() {
}

Chromix::ScriptingSupport::~ScriptingSupport() {
}

void Chromix::ScriptingSupport::setRenderCallback(WebKit::WebFrame* webFrame, v8::Handle<v8::Value> callbackFunction) {
    scriptState = WebCore::ScriptState::forContext(webFrame->mainWorldScriptContext());
    renderCallback = WebCore::ScriptValue(callbackFunction);
}

bool Chromix::ScriptingSupport::invokeRenderCallback(double time) {
    if (!scriptState)
        return false;
    // Need scope so we can construct Number
    WebCore::ScriptScope scope(scriptState);
    WebCore::ScriptCallback callback(scriptState, renderCallback);
    callback.appendArgument(WebCore::ScriptValue(v8::Number::New(time)));
    //XXX need to get exception details
    bool hadException = false;
    callback.call(hadException);
    return !hadException;
}
