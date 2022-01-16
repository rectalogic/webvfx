# Copyright (c) 2022 Andrew Wason, All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_executable(webvfx_driver
    driver.cpp
)
target_include_directories(webvfx_driver PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../..")
install(TARGETS webvfx_driver RUNTIME)