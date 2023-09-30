# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.16)

project(WebVfx VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)

if(NOT "${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "")
endif()

add_subdirectory(vfxpipe)
add_subdirectory(webvfx)

# add_subdirectory(viewer)
add_subdirectory(tools/driver)
add_subdirectory(plugins/mlt)
add_subdirectory(plugins/frei0r)
add_subdirectory(doc)

# add_dependencies(webvfx_viewer webvfx)
add_dependencies(webvfx vfxpipe)

if(MLT_FOUND)
    add_dependencies(mltwebvfx vfxpipe)
endif()

if(FREI0R_FOUND)
    add_dependencies(frei0r_plugin vfxpipe)
endif()
