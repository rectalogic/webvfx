# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(PkgConfig)

pkg_search_module(FREI0R IMPORTED_TARGET frei0r)

if(FREI0R_FOUND)
    add_library(vfxpipe_filter MODULE
        vfxpipe.cpp
        filter.cpp
    )
    add_library(vfxpipe_source MODULE
        vfxpipe.cpp
        source.cpp
    )
    add_library(vfxpipe_mixer2 MODULE
        vfxpipe.cpp
        mixer2.cpp
    )
    add_library(vfxpipe_mixer3 MODULE
        vfxpipe.cpp
        mixer3.cpp
    )

    set_target_properties(vfxpipe_filter PROPERTIES PREFIX "")
    set_target_properties(vfxpipe_source PROPERTIES PREFIX "")
    set_target_properties(vfxpipe_mixer2 PROPERTIES PREFIX "")
    set_target_properties(vfxpipe_mixer3 PROPERTIES PREFIX "")

    target_include_directories(vfxpipe_filter PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")
    target_include_directories(vfxpipe_source PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")
    target_include_directories(vfxpipe_mixer2 PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")
    target_include_directories(vfxpipe_mixer3 PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")

    target_compile_options(vfxpipe_filter PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(vfxpipe_source PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(vfxpipe_mixer2 PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(vfxpipe_mixer3 PUBLIC ${FREI0R_CFLAGS})

    install(TARGETS vfxpipe_filter LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS vfxpipe_source LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS vfxpipe_mixer2 LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS vfxpipe_mixer3 LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
endif()