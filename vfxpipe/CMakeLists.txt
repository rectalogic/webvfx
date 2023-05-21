# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(vfxpipe STATIC
    vfxpipe.cpp
)
set_target_properties(vfxpipe PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_include_directories(vfxpipe PUBLIC .)