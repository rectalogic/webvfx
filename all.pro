# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

isEmpty(PREFIX) {
    message("Install PREFIX not set, using default.")
}
include(common.pri)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += webvfx
SUBDIRS += viewer
SUBDIRS += tools/render

system(pkg-config --exists mlt-framework) {
    SUBDIRS += mlt
    mlt.depends = webvfx
} else {
    warning("MLT framework not found, skipping MLT plugin. Need to set PKG_CONFIG_PATH environment variable?")
}

viewer.depends = webvfx
render.depends = webvfx

# Documentation
doxydoc.target = doxydoc
doxydoc.commands = echo PROJECT_NUMBER=`git describe --always --dirty` | cat - doc/Doxyfile | doxygen -
doxydoc.depends = FORCE
QMAKE_EXTRA_TARGETS += doxydoc
