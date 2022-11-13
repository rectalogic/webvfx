# Copyright (c) 2022 Andrew Wason. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(PkgConfig)

pkg_search_module(FREI0R IMPORTED_TARGET frei0r)

if(FREI0R_FOUND)
    add_library(plugin STATIC
        plugin.cpp
    )

    add_library(vfxpipe_filter MODULE
        filter.cpp
    )
    add_library(vfxpipe_source MODULE
        source.cpp
    )
    add_library(vfxpipe_mixer2 MODULE
        mixer2.cpp
    )
    add_library(vfxpipe_mixer3 MODULE
        mixer3.cpp
    )

    set_target_properties(plugin PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
    set_target_properties(vfxpipe_filter PROPERTIES PREFIX "")
    set_target_properties(vfxpipe_source PROPERTIES PREFIX "")
    set_target_properties(vfxpipe_mixer2 PROPERTIES PREFIX "")
    set_target_properties(vfxpipe_mixer3 PROPERTIES PREFIX "")

    target_include_directories(plugin PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")

    target_compile_options(plugin PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(vfxpipe_filter PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(vfxpipe_source PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(vfxpipe_mixer2 PUBLIC ${FREI0R_CFLAGS})
    target_compile_options(vfxpipe_mixer3 PUBLIC ${FREI0R_CFLAGS})

    target_link_libraries(vfxpipe_filter plugin vfxpipe)
    target_link_libraries(vfxpipe_source plugin vfxpipe)
    target_link_libraries(vfxpipe_mixer2 plugin vfxpipe)
    target_link_libraries(vfxpipe_mixer3 plugin vfxpipe)

    install(TARGETS vfxpipe_filter LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS vfxpipe_source LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS vfxpipe_mixer2 LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
    install(TARGETS vfxpipe_mixer3 LIBRARY DESTINATION ${FREI0R_LIBDIR}/frei0r-1/webvfx)
endif()