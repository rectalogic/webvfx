# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.21)

project(WebFfx VERSION 1.0.0 LANGUAGES CXX)

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

add_subdirectory(webvfx)
add_subdirectory(frameserver)
add_subdirectory(viewer)
add_subdirectory(tools/render)
add_subdirectory(mlt)

add_dependencies(webvfx_viewer webvfx)
add_dependencies(webvfx_frameserver webvfx)
add_dependencies(webvfx_render webvfx)

# Move these from qmake to cmake https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
# https://cmake.org/cmake/help/latest/module/FindDoxygen.html?highlight=doxygen#module:FindDoxygen
# Documentation
# doxydoc.target = doxydoc
# doxydoc.commands = echo PROJECT_NUMBER=`git describe --always --dirty` | cat - doc/Doxyfile | doxygen -
# doxydoc.depends = FORCE
# QMAKE_EXTRA_TARGETS += doxydoc