// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_CHROMIX_H_
#define CHROMIX_CHROMIX_H_

#include <chromix/mix_kit.h>
#include <chromix/logger.h>

namespace Chromix {

// initialize must only be called once.
// argv must be valid until shutdown() is called.
// argc must be > 0 and argv must have at least an empty string.
// argv may be modified.
// A Logger implementation may be supplied, it will be deleted at shutdown.
bool initialize(int argc, char* argv[], Logger* logger=0);

MixKit* createMixKit();

void shutdown();

}
#endif
