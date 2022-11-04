// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <frei0r.h>
}

void f0r_get_plugin_info(f0r_plugin_info_t* info)
{
    info->name = "VfxPipe Filter";
    info->author = "Andrew Wason";
    info->plugin_type = F0R_PLUGIN_TYPE_FILTER;
    info->color_model = F0R_COLOR_MODEL_RGBA8888;
    info->frei0r_version = FREI0R_MAJOR_VERSION;
    info->major_version = 0;
    info->minor_version = 9;
    info->num_params = 1;
    info->explanation = "Pipes video frames to an external filter command";
}