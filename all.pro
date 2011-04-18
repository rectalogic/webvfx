# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

isEmpty(PREFIX) {
    message("PREFIX not set, using default.")
}
include(common.pri)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += webvfx
SUBDIRS += viewer
SUBDIRS += demo
SUBDIRS += tools/browser

unix:system(pkg-config --exists mlt-framework) {
    SUBDIRS += mlt
    mlt.depends = webvfx
} else {
    message("MLT framework not found, skipping MLT plugin. Need to set PKG_CONFIG_PATH?")
}

viewer.depends = webvfx
demo.depends = webvfx

# Documentation
doxydoc.target = doxydoc
doxydoc.commands = echo PROJECT_NUMBER=`git describe --always --dirty` | cat - doc/Doxyfile | doxygen -
doxydoc.depends = FORCE
QMAKE_EXTRA_TARGETS += doxydoc
