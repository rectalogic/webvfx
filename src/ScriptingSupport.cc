#include "ScriptingSupport.h"

#include <map>
#include <base/singleton.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptScope.h>
#include <third_party/WebKit/WebCore/bindings/v8/ScriptFunctionCall.h>

typedef std::map<WebKit::WebView*, Chromix::ScriptingSupport*> ViewScriptMap;


Chromix::ScriptingSupport::ScriptingSupport() {
}

Chromix::ScriptingSupport::~ScriptingSupport() {
    // Remove from map
    if (this->webView)
        Singleton<ViewScriptMap>::get()->erase(this->webView);
}

/*static*/
Chromix::ScriptingSupport* Chromix::ScriptingSupport::fromWebView(WebKit::WebView* webView) {
    ViewScriptMap* views = Singleton<ViewScriptMap>::get();
    ViewScriptMap::iterator it = views->find(webView);
    return it == views->end() ? NULL : it->second;
}

void Chromix::ScriptingSupport::initialize(WebKit::WebView* webView) {
    this->webView = webView;

    // Register in map
    Singleton<ViewScriptMap>::get()->insert(std::make_pair(webView, this));

    v8::HandleScope scope;
    scriptState = WebCore::ScriptState::forContext(webView->mainFrame()->mainWorldScriptContext());
}

void Chromix::ScriptingSupport::setRenderCallback(v8::Handle<v8::Value> callbackFunction) {
    renderCallback = WebCore::ScriptValue(callbackFunction);
}

bool Chromix::ScriptingSupport::invokeRenderCallback(double time) {
    if (!scriptState)
        return false;
    // Need scope so we can construct Number
    WebCore::ScriptScope scope(scriptState);
    WebCore::ScriptCallback callback(scriptState, renderCallback);
    callback.appendArgument(WebCore::ScriptValue(v8::Number::New(time)));

    bool hadException = false;
    callback.call(hadException);
    return !hadException;
}

