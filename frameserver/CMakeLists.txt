# Copyright (c) 2022 Andrew Wason, All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(Qt6 COMPONENTS Gui Quick REQUIRED)

set(CMAKE_AUTOMOC ON)

qt_add_executable(webvfx_frameserver
    main.cpp
    frameserver.cpp
)
set_property(TARGET webvfx_frameserver PROPERTY AUTOMOC ON)

target_include_directories(webvfx_frameserver PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")
target_link_libraries(webvfx_frameserver PRIVATE Qt6::Gui Qt6::Quick webvfx)

install(TARGETS webvfx_frameserver RUNTIME DESTINATION)