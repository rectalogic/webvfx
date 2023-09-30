# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(PkgConfig)

pkg_search_module(MLT IMPORTED_TARGET mlt-framework-7>=7.4)

if(MLT_FOUND)
    add_library(mltwebvfx MODULE
        factory.cpp
        service_locker.cpp
        service_manager.cpp
        filter.cpp
        producer.cpp
        transition.cpp
    )

    target_include_directories(mltwebvfx PRIVATE ${MLT_INCLUDE_DIRS} "${CMAKE_CURRENT_LIST_DIR}/../..")
    target_compile_options(mltwebvfx PUBLIC ${MLT_CFLAGS})
    target_link_libraries(mltwebvfx PRIVATE PkgConfig::MLT vfxpipe)

    pkg_get_variable(MLT_INSTALL_MODULE_DIR ${MLT_MODULE_NAME} moduledir)
    install(TARGETS mltwebvfx LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})

    # https://stackoverflow.com/questions/66550128/create-symlinks-to-libraries-in-different-folder-during-installation-with-cmake
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/webvfx ${MLT_INSTALL_MODULE_DIR}/webvfx)")
endif()