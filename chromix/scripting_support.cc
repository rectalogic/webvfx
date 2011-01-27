// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/scripting_support.h"
#include "chromix/delegate.h"

#include <map>
#include <base/lazy_instance.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/Source/WebCore/bindings/v8/ScriptScope.h>
#include <third_party/WebKit/Source/WebCore/bindings/v8/ScriptFunctionCall.h>

typedef std::map<WebKit::WebView*, Chromix::ScriptingSupport*> ViewScriptMap;
static base::LazyInstance<ViewScriptMap> g_view_script_map(base::LINKER_INITIALIZED);

Chromix::ScriptingSupport::ScriptingSupport(Delegate* delegate) : delegate(delegate) {
}

Chromix::ScriptingSupport::~ScriptingSupport() {
    // Remove from map
    if (this->webView)
        g_view_script_map.Get().erase(this->webView);
}

/*static*/
Chromix::ScriptingSupport* Chromix::ScriptingSupport::fromWebView(WebKit::WebView* webView) {
    ViewScriptMap* views = g_view_script_map.Pointer();
    ViewScriptMap::iterator it = views->find(webView);
    return it == views->end() ? NULL : it->second;
}

void Chromix::ScriptingSupport::initialize(WebKit::WebView* webView) {
    this->webView = webView;

    // Register in map
    g_view_script_map.Get().insert(std::make_pair(webView, this));

    v8::HandleScope scope;
    scriptState = WebCore::ScriptState::forContext(webView->mainFrame()->mainWorldScriptContext());
}

v8::Handle<v8::Value> Chromix::ScriptingSupport::getParameterValue(const std::string& name) {
    if (delegate)
        return delegate->getParameterValue(name);
    return v8::Undefined();

}

void Chromix::ScriptingSupport::setRenderCallback(v8::Handle<v8::Value> callbackFunction) {
    renderCallback = WebCore::ScriptValue(callbackFunction);
}

bool Chromix::ScriptingSupport::invokeRenderCallback(double time) {
    if (!scriptState)
        return false;
    if (renderCallback.hasNoValue()) {
        if (delegate)
            delegate->logMessage("Render callback not set - loaded page must call chromix.setRenderCallback");
        return false;
    }

    // Need scope so we can construct Number
    WebCore::ScriptScope scope(scriptState);
    WebCore::ScriptCallback callback(scriptState, renderCallback);
    callback.appendArgument(WebCore::ScriptValue(v8::Number::New(time)));

    bool hadException = false;
    callback.call(hadException);
    if (hadException && delegate)
        delegate->logMessage(std::string("Render callback threw exception: ") + *v8::String::Utf8Value(scriptState->exception()->ToString()));
    return !hadException;
}

