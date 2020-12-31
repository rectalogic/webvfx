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
LIBS += -L$$DESTDIR -lwebvfx

QMAKE_RPATHDIR += $$PREFIX/lib

target.path = $$PREFIX/bin
INSTALLS += target
