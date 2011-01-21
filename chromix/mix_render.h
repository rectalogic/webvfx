// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_MIX_RENDER_H_
#define CHROMIX_MIX_RENDER_H_

#include "chromix/loader.h"

#include <base/basictypes.h>
#include <skia/ext/platform_canvas.h>
#include <third_party/WebKit/WebKit/chromium/public/WebSize.h>


namespace Chromix {

class ScriptingSupport;

class MixRender {
public:
    // MixRender takes ownership of Delegate (may be NULL)
    MixRender(Delegate* delegate);
    virtual ~MixRender();

    bool loadURL(const std::string& url);
    void resize(int width, int height);
    const SkBitmap* render(double time);

    unsigned char* writeableDataForImageParameter(const std::string& name, unsigned int width, unsigned int height);

private:
    Delegate* delegate;
    WebKit::WebView *webView;
    WebKit::WebSize size;
    skia::PlatformCanvas* skiaCanvas;
    Loader loader;
    ScriptingSupport *scriptingSupport;

    DISALLOW_COPY_AND_ASSIGN(MixRender);
};

}

#endif
