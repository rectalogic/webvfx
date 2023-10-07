// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin.h" // for get_common_plugin_info
#include <frei0r.h> // for f0r_plugin_info_t, F0R_PLUGIN_TYPE_SOURCE, f0r_get_plugin_info

void f0r_get_plugin_info(f0r_plugin_info_t* info)
{
    get_common_plugin_info(info);
    info->name = "WebVfx Source";
    info->plugin_type = F0R_PLUGIN_TYPE_SOURCE;
    info->explanation = "Pipes video frames from WebVfx source command";
}