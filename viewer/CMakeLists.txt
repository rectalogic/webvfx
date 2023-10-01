# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(Qt6 REQUIRED QUIET COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(webvfx_viewer MACOSX_BUNDLE
    main.cpp
    image_color.cpp
    content_pipe.cpp
    viewer.cpp
    viewer.ui
)

target_link_libraries(webvfx_viewer PRIVATE Qt6::Widgets vfxpipe)

if(APPLE)
    set_target_properties(webvfx_viewer PROPERTIES OUTPUT_NAME "WebVfx Viewer")
endif()

install(TARGETS webvfx_viewer
    RUNTIME DESTINATION
    BUNDLE DESTINATION bin
)
