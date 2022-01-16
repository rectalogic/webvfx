# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(PkgConfig)

pkg_search_module(MLT IMPORTED_TARGET mlt-framework-7>=7.4)

if(MLT_FOUND)
    add_library(vfxpipe MODULE
        factory.cpp
        service_locker.cpp
        service_manager.cpp
        vfxpipe_filter.cpp
        vfxpipe_producer.cpp
        vfxpipe_transition.cpp
    )

    target_include_directories(vfxpipe PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")
    target_include_directories(vfxpipe PRIVATE ${MLT_INCLUDE_DIRS})
    target_compile_options(vfxpipe PUBLIC ${MLT_CFLAGS})
    target_link_libraries(vfxpipe PRIVATE PkgConfig::MLT)

    pkg_get_variable(MLT_INSTALL_MODULE_DIR ${MLT_MODULE_NAME} moduledir)
    install(TARGETS vfxpipe LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})
endif()