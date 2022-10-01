# Copyright (c) 2022 Andrew Wason, All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(Doxygen QUIET)

if (DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_LOGO "logo.png")
    set(DOXYGEN_EXAMPLE_PATH "demo LICENSE")
    set(DOXYGEN_HTML_FOOTER "footer.html")

    doxygen_add_docs(docs
        ${PROJECT_SOURCE_DIR}/doc
        ${PROJECT_SOURCE_DIR}/webvfx
    )
endif()