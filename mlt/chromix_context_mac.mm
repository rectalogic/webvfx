// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix_context.h"
#import <Foundation/Foundation.h>

ChromixContext::ChromixContext() {
    context = [[NSAutoreleasePool alloc] init];
}

ChromixContext::~ChromixContext() {
    [(NSAutoreleasePool*)context drain];
}
