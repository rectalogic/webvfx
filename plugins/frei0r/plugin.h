// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
#include <frei0r.h> // for f0r_plugin_info_t
}

void get_common_plugin_info(f0r_plugin_info_t* info);