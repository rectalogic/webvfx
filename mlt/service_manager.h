// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBFX_EFFECTS_MANAGER_H_
#define MLTWEBFX_EFFECTS_MANAGER_H_

extern "C" {
    #include <mlt/framework/mlt_service.h>
}

namespace WebFX
{
    class WebEffects;
}

class ServiceManager
{
public:
    ServiceManager(mlt_service service);
    ~ServiceManager();
    bool initialize(const char* url, int width, int height);
    WebFX::WebEffects* getWebEffects();

private:
    static const char* kWebFXPropertyName;
    static void destroyWebEffects(WebFX::WebEffects* webEffects);
    mlt_service service;
    WebFX::WebEffects* webEffects;
};

#endif
