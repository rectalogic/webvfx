# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(PkgConfig)

pkg_search_module(FREI0R IMPORTED_TARGET frei0r)

if(FREI0R_FOUND)
    add_library(frei0r_plugin STATIC
        plugin.cpp
    )

    add_library(webvfx_filter MODULE
        filter.cpp
    )
    add_library(webvfx_source MODULE
        source.cpp
    )
    add_library(webvfx_mixer2 MODULE
        mixer2.cpp
    )
    add_library(webvfx_mixer3 MODULE
        mixer3.cpp
    )

    set_target_properties(frei0r_plugin PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
    set_target_properties(webvfx_filter PROPERTIES PREFIX "")
    set_target_properties(webvfx_source PROPERTIES PREFIX "")
    set_target_properties(webvfx_mixer2 PROPERTIES PREFIX "")
    set_target_properties(webvfx_mixer3 PROPERTIES PREFIX "")

    target_include_directories(frei0r_plugin PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../..")

    target_compile_options(frei0r_plugin PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(webvfx_filter PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(webvfx_source PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(webvfx_mixer2 PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(webvfx_mixer3 PUBLIC ${FREI0R_CFLAGS})

    target_link_libraries(frei0r_plugin vfxpipe)
    target_link_libraries(webvfx_filter frei0r_plugin)
    target_link_libraries(webvfx_source frei0r_plugin)
    target_link_libraries(webvfx_mixer2 frei0r_plugin)
    target_link_libraries(webvfx_mixer3 frei0r_plugin)

    install(TARGETS webvfx_filter LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS webvfx_source LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS webvfx_mixer2 LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS webvfx_mixer3 LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)

    # https://stackoverflow.com/questions/66550128/create-symlinks-to-libraries-in-different-folder-during-installation-with-cmake
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/webvfx ${FREI0R_LIBDIR}/frei0r-1/webvfx/webvfx)")
endif()