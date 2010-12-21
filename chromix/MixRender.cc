// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/MixRender.h"
#include "chromix/ScriptingSupport.h"
#include "chromix/ParameterMap.h"

#include <base/string16.h>
#include <third_party/WebKit/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURL.h>
#include <third_party/WebKit/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRect.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSettings.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>
#include <webkit/glue/webkit_glue.h>
#include <skia/ext/bitmap_platform_device.h>


Chromix::MixRender::MixRender() :
    size(),
    skiaCanvas(NULL),
    loader(),
    scriptingSupport(new Chromix::ScriptingSupport())
{
    webView = WebKit::WebView::create(&loader, NULL);

    WebKit::WebSettings *settings = webView->settings();
    settings->setStandardFontFamily(WebKit::WebString("Times New Roman"));
    settings->setFixedFontFamily(WebKit::WebString("Courier New"));
    settings->setSerifFontFamily(WebKit::WebString("Times New Roman"));
    settings->setSansSerifFontFamily(WebKit::WebString("Arial"));
    settings->setCursiveFontFamily(WebKit::WebString("Comic Sans MS"));
    settings->setFantasyFontFamily(WebKit::WebString("Times New Roman"));
    settings->setDefaultFontSize(16);
    settings->setDefaultFixedFontSize(13);
    settings->setMinimumFontSize(1);
    settings->setJavaScriptEnabled(true);
    settings->setJavaScriptCanOpenWindowsAutomatically(false);
    settings->setLoadsImagesAutomatically(true);
    settings->setImagesEnabled(true);
    settings->setPluginsEnabled(true);
    settings->setDOMPasteAllowed(false);
    settings->setJavaEnabled(false);
    settings->setJavaEnabled(false);
    settings->setAllowScriptsToCloseWindows(false);
    settings->setUsesPageCache(false);
    settings->setShouldPaintCustomScrollbars(false);
    settings->setAllowUniversalAccessFromFileURLs(true);
    settings->setAllowFileAccessFromFileURLs(true);
    settings->setExperimentalWebGLEnabled(true);
    //XXX see http://groups.google.com/a/chromium.org/group/chromium-dev/browse_thread/thread/26ce7cbbc758a504/3515ab0391212a37?show_docid=3515ab0391212a37
    settings->setAcceleratedCompositingEnabled(false);//XXX crashes WebGL
    settings->setAccelerated2dCanvasEnabled(true);

    webView->initializeMainFrame(&loader);
    //webView->mainFrame()->setCanHaveScrollbars(false);

    scriptingSupport->initialize(webView);
}

Chromix::MixRender::~MixRender() {
    delete skiaCanvas;
    delete scriptingSupport;
    webView->close();
}

bool Chromix::MixRender::loadURL(const std::string& url) {
    return loader.loadURL(webView, url) && scriptingSupport->hasRenderCallback();
}

void Chromix::MixRender::resize(int width, int height) {
    if (size.width == width && size.height == height)
        return;
    size.width = width;
    size.height = height;
    webView->resize(size);
    // NULL out so we recreate canvas in render()
    delete skiaCanvas;
    skiaCanvas = NULL;
}

const SkBitmap* Chromix::MixRender::render(double time) {
    if (skiaCanvas == NULL)
        skiaCanvas = new skia::PlatformCanvas(size.width, size.height, true);

    //XXX set flag allowing image param access (so only allowed while rendering)
    if (!scriptingSupport->invokeRenderCallback(time))
        return NULL;

    webView->layout();
    webView->paint(webkit_glue::ToWebCanvas(skiaCanvas), WebKit::WebRect(0, 0, size.width, size.height));

    // Get canvas bitmap
    // XXX return wrapper class that internally does SkAutoLockPixels
    skia::BitmapPlatformDevice &skiaDevice = static_cast<skia::BitmapPlatformDevice&>(skiaCanvas->getTopPlatformDevice());
    return &skiaDevice.accessBitmap(false);
}

void Chromix::MixRender::setParameterValue(const std::string& name, bool value) {
    scriptingSupport->getParameterMap().setParameterValue(name, Chromix::BooleanParameterValue::create(value));
}

void Chromix::MixRender::setParameterValue(const std::string& name, double value) {
    scriptingSupport->getParameterMap().setParameterValue(name, Chromix::NumberParameterValue::create(value));
}

void Chromix::MixRender::setParameterValue(const std::string& name, const std::string& value) {
    scriptingSupport->getParameterMap().setParameterValue(name, Chromix::StringParameterValue::create(value));
}

unsigned char* Chromix::MixRender::writeableDataForImageParameter(const std::string& name, unsigned int width, unsigned int height) {
    return scriptingSupport->getParameterMap().writeableDataForImageParameter(name, width, height);
}

