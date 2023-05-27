# Copyright (c) 2022 Andrew Wason, All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(Qt6 REQUIRED QUIET COMPONENTS Core Gui Multimedia Qml Quick Quick3D OPTIONAL_COMPONENTS WebEngineQuick)

set(CMAKE_AUTOMOC ON)

qt_add_executable(webvfx
    main.cpp
    frameserver.cpp
    content_context.cpp
    parameters.cpp
    qml_content.cpp
    video_texture.cpp
    webvfx.cpp
)
set_property(TARGET webvfx PROPERTY AUTOMOC ON)

list(APPEND FrameserverLibraries Qt6::Core Qt6::Gui Qt6::Multimedia Qt6::GuiPrivate Qt6::Qml Qt6::Quick Qt6::QuickPrivate Qt6::Quick3D)

if(${Qt6WebEngineQuick_FOUND})
    list(APPEND FrameserverLibraries Qt6::WebEngineQuick)

    # Disable for now
    # add_definitions(-DWEBENGINEQUICK)
endif()

target_link_libraries(webvfx PRIVATE ${FrameserverLibraries} vfxpipe)

qt_policy(SET QTP0001 NEW)
qt_add_qml_module(webvfx
    URI org.webvfx.WebVfx.native
    VERSION 1.0
    NO_PLUGIN
)

install(TARGETS webvfx RUNTIME DESTINATION)