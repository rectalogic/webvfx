// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/mix_render.h"
#include "chromix/scripting_support.h"
#include "chromix/image_map.h"
#include "chromix/delegate.h"

#include <base/string16.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebView.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebURL.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebURLRequest.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebRect.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebSettings.h>
#include <third_party/WebKit/Source/JavaScriptCore/wtf/text/WTFString.h>
#include <webkit/glue/webkit_glue.h>
#include <skia/ext/bitmap_platform_device.h>


Chromix::MixRender::MixRender(Delegate* delegate) :
    delegate(delegate),
    size(),
    skiaCanvas(NULL),
    loader(delegate),
    scriptingSupport(new Chromix::ScriptingSupport(delegate))
{
    webView = WebKit::WebView::create(&loader, NULL, NULL);

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
    //XXX security issue? should we check command_line.HasSwitch(switches::kAllowFileAccessFromFiles)
    //XXX http://code.google.com/p/chromium/issues/detail?id=47416
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
    delete delegate;
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

unsigned char* Chromix::MixRender::writeableDataForImageParameter(const std::string& name, unsigned int width, unsigned int height) {
    return scriptingSupport->writeableDataForImageParameter(name, width, height);
}

