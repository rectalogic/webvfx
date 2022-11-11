# Copyright (c) 202 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(Qt6 COMPONENTS Core Gui Multimedia Qml Quick Quick3D REQUIRED)

set(SOURCES
    content_context.cpp
    logger.cpp
    parameters.cpp
    qml_content.cpp
    video_texture.cpp
    webvfx.cpp
)
qt_add_resources(SOURCES resources/resources.qrc)

qt_add_library(webvfx SHARED ${SOURCES})
set_property(TARGET webvfx PROPERTY AUTOMOC ON)
set_property(TARGET webvfx PROPERTY AUTORCC ON)

target_include_directories(webvfx PUBLIC "${CMAKE_CURRENT_LIST_DIR}/..")

qt_add_qml_module(webvfx
    URI org.webvfx.WebVfx.native
    VERSION 1.0
    NO_PLUGIN
)

target_link_libraries(webvfx PUBLIC Qt6::Core Qt6::Gui Qt6::Multimedia Qt6::GuiPrivate Qt6::Qml Qt6::Quick Qt6::QuickPrivate Qt6::Quick3D)

install(TARGETS webvfx LIBRARY)
