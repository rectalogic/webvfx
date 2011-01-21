// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_EXTENSION_H_
#define CHROMIX_EXTENSION_H_

namespace v8 {
    class Extension;
}

namespace Chromix {

class ChromixExtension {
public:
    static v8::Extension* Get();
};

}
#endif
