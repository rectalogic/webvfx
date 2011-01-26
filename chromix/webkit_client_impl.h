// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_WEBKIT_CLIENT_IMPL_H_
#define CHROMIX_WEBKIT_CLIENT_IMPL_H_

#include <webkit/glue/simple_webmimeregistry_impl.h>
#include <webkit/glue/webfileutilities_impl.h>
#include <webkit/glue/webclipboard_impl.h>
#include <webkit/glue/webkitclient_impl.h>
#include <webkit/gpu/webgraphicscontext3d_in_process_impl.h>

namespace Chromix {

class WebKitClientImpl : public webkit_glue::WebKitClientImpl {
public:
    WebKitClientImpl() {};
    virtual ~WebKitClientImpl() {};

    virtual WebKit::WebMimeRegistry* mimeRegistry() { return &mimeRegistryImpl; }
    virtual WebKit::WebClipboard* clipboard() { return &clipboardImpl; }
    virtual WebKit::WebFileUtilities* fileUtilities() { return &fileUtiliesImpl; }
    virtual WebKit::WebGraphicsContext3D* createGraphicsContext3D() {
        return new webkit::gpu::WebGraphicsContext3DInProcessImpl();
    }

private:
    webkit_glue::SimpleWebMimeRegistryImpl mimeRegistryImpl;
    webkit_glue::WebClipboardImpl clipboardImpl;
    webkit_glue::WebFileUtilitiesImpl fileUtiliesImpl;
};

}
#endif

