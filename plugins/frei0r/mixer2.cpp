// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin.h"
#include <frei0r.h>

void f0r_get_plugin_info(f0r_plugin_info_t* info)
{
    get_common_plugin_info(info);
    info->name = "WebVfx Mixer2";
    info->plugin_type = F0R_PLUGIN_TYPE_MIXER2;
    info->explanation = "Pipes video frames to WebVfx mixer2 command";
}