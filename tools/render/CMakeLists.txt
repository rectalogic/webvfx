# Copyright (c) 2022 Andrew Wason, All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(Qt6 COMPONENTS Qml Quick REQUIRED)

qt_add_executable(webvfx_render
    render.cpp
)
target_link_libraries(webvfx_render PRIVATE Qt6::Qml Qt6::Quick webvfx)

install(TARGETS webvfx_render RUNTIME)