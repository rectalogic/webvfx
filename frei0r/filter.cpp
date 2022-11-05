// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfxpipe.h"

void f0r_get_plugin_info(f0r_plugin_info_t* info)
{
    get_common_plugin_info(info);
    info->name = "VfxPipe Filter";
    info->plugin_type = F0R_PLUGIN_TYPE_FILTER;
    info->explanation = "Pipes video frames to an external filter command";
}