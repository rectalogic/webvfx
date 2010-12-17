// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>

void* chromix_context_new() {
    return [[NSAutoreleasePool alloc] init];
}

void chromix_context_close(void* pool) {
    [(NSAutoreleasePool*)pool drain];
}
