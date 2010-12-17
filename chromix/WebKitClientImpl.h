// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_WEBKITCLIENTIMPL_H_
#define CHROMIX_WEBKITCLIENTIMPL_H_

#include <third_party/WebKit/WebKit/chromium/public/WebGraphicsContext3D.h>
#include <webkit/glue/simple_webmimeregistry_impl.h>
#include <webkit/glue/webfileutilities_impl.h>
#include <webkit/glue/webclipboard_impl.h>
#include <webkit/glue/webkitclient_impl.h>

namespace Chromix {

class WebKitClientImpl : public webkit_glue::WebKitClientImpl {
public:
    WebKitClientImpl() {};
    virtual ~WebKitClientImpl() {};

    virtual WebKit::WebMimeRegistry* mimeRegistry() { return &mimeRegistryImpl; }
    virtual WebKit::WebClipboard* clipboard() { return &clipboardImpl; }
    virtual WebKit::WebFileUtilities* fileUtilities() { return &fileUtiliesImpl; }
    virtual WebKit::WebGraphicsContext3D* createGraphicsContext3D() {
        return WebKit::WebGraphicsContext3D::createDefault();
    }

private:
    webkit_glue::SimpleWebMimeRegistryImpl mimeRegistryImpl;
    webkit_glue::WebClipboardImpl clipboardImpl;
    webkit_glue::WebFileUtilitiesImpl fileUtiliesImpl;
};

}
#endif

