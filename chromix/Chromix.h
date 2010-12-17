// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_CHROMIX_H_
#define CHROMIX_CHROMIX_H_

namespace Chromix {
enum InitializeResult { InitializeFailure, InitializeSuccess, InitializeAlready };

// Chromix must be initialized, used, and shutdown from the same thread
InitializeResult initialize(int argc=0, const char* argv[]=0);
void shutdown();
}
#endif
