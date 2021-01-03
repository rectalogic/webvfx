# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../common.pri)
TEMPLATE = app

SOURCES += main.cpp
SOURCES += image_color.cpp
SOURCES += viewer.cpp
HEADERS += image_color.h
HEADERS += viewer.h
FORMS += viewer.ui

QT += widgets quick
!mac:TARGET = webvfx_viewer
mac:TARGET = "WebVfx Viewer"

DEPENDPATH += $$PWD/../webvfx
CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../webvfx/release/ -lwebvfx
else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../webvfx/debug/ -lwebvfx

QMAKE_RPATHDIR += $$[QT_INSTALL_LIBS]

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
